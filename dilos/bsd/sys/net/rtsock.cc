/*
 * Copyright (c) 1988, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)rtsock.c	8.7 (Berkeley) 10/12/95
 * $FreeBSD$
 */

#include <osv/initialize.hh>
#include <bsd/porting/netport.h>

#include <bsd/sys/sys/param.h>
#include <bsd/sys/sys/domain.h>
#include <bsd/sys/sys/mbuf.h>
#include <bsd/sys/sys/priv.h>
#include <bsd/sys/sys/protosw.h>
#include <bsd/sys/sys/socket.h>
#include <bsd/sys/sys/socketvar.h>
#include <bsd/sys/sys/sysctl.h>

#include <bsd/sys/net/if.h>
#include <bsd/sys/net/if_dl.h>
#include <bsd/sys/net/if_llatbl.h>
#include <bsd/sys/net/if_types.h>
#include <bsd/sys/net/netisr.h>
#include <bsd/sys/net/raw_cb.h>
#include <bsd/sys/net/route.h>
#include <bsd/sys/net/vnet.h>

#include <bsd/sys/netinet/in.h>
#include <bsd/sys/netinet/if_ether.h>
#ifdef INET6
#include <bsd/sys/netinet6/scope6_var.h>
#endif

#if !defined(offsetof)
#define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#endif

#if defined(INET) || defined(INET6)
#ifdef SCTP
extern void sctp_addr_change(struct bsd_ifaddr *ifa, int cmd);
#endif /* SCTP */
#endif

MALLOC_DEFINE(M_RTABLE, "routetbl", "routing tables");

/* NB: these are not modified */
static struct	bsd_sockaddr route_src = { 2, PF_ROUTE, };
static struct	bsd_sockaddr sa_zero   = { sizeof(sa_zero), AF_INET, };

/*
 * Used by rtsock/raw_input callback code to decide whether to filter the update
 * notification to a socket bound to a particular FIB.
 */
#define	RTS_FILTER_FIB	M_PROTO8
#define	RTS_ALLFIBS	-1

static struct {
	int	ip_count;	/* attached w/ AF_INET */
	int	ip6_count;	/* attached w/ AF_INET6 */
	int	ipx_count;	/* attached w/ AF_IPX */
	int	any_count;	/* total attached */
} route_cb;

mutex rtsock_mtx;

#if 0
MTX_SYSINIT(rtsock, &rtsock_mtx, "rtsock route_cb lock", MTX_DEF);
#endif

#define	RTSOCK_LOCK()	mutex_lock(&rtsock_mtx)
#define	RTSOCK_UNLOCK()	mutex_unlock(&rtsock_mtx)
#define	RTSOCK_LOCK_ASSERT()	assert(rtsock_mtx.owned())

#if 0
SYSCTL_NODE(_net, OID_AUTO, route, CTLFLAG_RD, 0, "");
#endif

struct walkarg {
	int	w_tmemsize;
	int	w_op, w_arg;
	caddr_t	w_tmem;
	struct sysctl_req *w_req;
};

static void	rts_input(struct mbuf *m);
static struct mbuf *rt_msg1(int type, struct rt_addrinfo *rtinfo);
static int	rt_msg2(int type, struct rt_addrinfo *rtinfo,
			caddr_t cp, struct walkarg *w);
static int	rt_xaddrs(caddr_t cp, caddr_t cplim,
			struct rt_addrinfo *rtinfo);
static int	route_output(struct mbuf *m, struct socket *so);
static void	rt_setmetrics(u_long which, const struct rt_metrics *in,
			struct rt_metrics_lite *out);
static void	rt_getmetrics(const struct rt_metrics_lite *in,
			struct rt_metrics *out);
static void	rt_dispatch(struct mbuf *, bsd_sa_family_t);

static struct netisr_handler rtsock_nh = initialize_with([] (netisr_handler& x) {
	x.nh_name = "rtsock";
	x.nh_handler = rts_input;
	x.nh_proto = NETISR_ROUTE;
	x.nh_policy = NETISR_POLICY_SOURCE;
});

#if 0
static int
sysctl_route_netisr_maxqlen(SYSCTL_HANDLER_ARGS)
{
	int error, qlimit;

	netisr_getqlimit(&rtsock_nh, &qlimit);
	error = sysctl_handle_int(oidp, &qlimit, 0, req);
        if (error || !req->newptr)
                return (error);
	if (qlimit < 1)
		return (EINVAL);
	return (netisr_setqlimit(&rtsock_nh, qlimit));
}
SYSCTL_PROC(_net_route, OID_AUTO, netisr_maxqlen, CTLTYPE_INT|CTLFLAG_RW,
    0, 0, sysctl_route_netisr_maxqlen, "I",
    "maximum routing socket dispatch queue length");
#endif

void
rts_init(void)
{
#if 0
    int tmp;
	if (TUNABLE_INT_FETCH("net.route.netisr_maxqlen", &tmp))
		rtsock_nh.nh_qlimit = tmp;
#endif
	netisr_register(&rtsock_nh);
}
SYSINIT(rtsock, SI_SUB_PROTO_DOMAIN, SI_ORDER_THIRD, rts_init, 0);

static int
raw_input_rts_cb(struct mbuf *m, struct sockproto *proto, struct bsd_sockaddr *src,
    struct rawcb *rp)
{
	int fibnum;

	KASSERT(m != NULL, ("%s: m is NULL", __func__));
	KASSERT(proto != NULL, ("%s: proto is NULL", __func__));
	KASSERT(rp != NULL, ("%s: rp is NULL", __func__));

	/* No filtering requested. */
	if ((m->m_hdr.mh_flags & RTS_FILTER_FIB) == 0)
		return (0);

	/* Check if it is a rts and the fib matches the one of the socket. */
	fibnum = M_GETFIB(m);
	if (proto->sp_family != PF_ROUTE ||
	    rp->rcb_socket == NULL ||
	    rp->rcb_socket->so_fibnum == fibnum)
		return (0);

	/* Filtering requested and no match, the socket shall be skipped. */
	return (1);
}

static void
rts_input(struct mbuf *m)
{
	struct sockproto route_proto;
	unsigned short *family;
	struct m_tag *tag;

	route_proto.sp_family = PF_ROUTE;
	tag = m_tag_find(m, PACKET_TAG_RTSOCKFAM, NULL);
	if (tag != NULL) {
		family = (unsigned short *)(tag + 1);
		route_proto.sp_protocol = *family;
		m_tag_delete(m, tag);
	} else
		route_proto.sp_protocol = 0;

	raw_input_ext(m, &route_proto, &route_src, raw_input_rts_cb);
}

/*
 * It really doesn't make any sense at all for this code to share much
 * with raw_usrreq.c, since its functionality is so restricted.  XXX
 */
static void
rts_abort(struct socket *so)
{

	raw_usrreqs.pru_abort(so);
}

static void
rts_close(struct socket *so)
{

	raw_usrreqs.pru_close(so);
}

/* pru_accept is EOPNOTSUPP */

static int
rts_attach(struct socket *so, int proto, struct thread *td)
{
	struct rawcb *rp;
	int s, error;

	KASSERT(so->so_pcb == NULL, ("rts_attach: so_pcb != NULL"));

	/* XXX */
	rp = (rawcb *)malloc(sizeof *rp);
	if (rp == NULL)
		return ENOBUFS;
	bzero(rp, sizeof *rp);

	/*
	 * The splnet() is necessary to block protocols from sending
	 * error notifications (like RTM_REDIRECT or RTM_LOSING) while
	 * this PCB is extant but incompletely initialized.
	 * Probably we should try to do more of this work beforehand and
	 * eliminate the spl.
	 */
	s = splnet();
	so->so_pcb = (caddr_t)rp;
	so->set_mutex(&rtsock_mtx);
	so->so_fibnum = 0;
	error = raw_attach(so, proto);
	rp = sotorawcb(so);
	if (error) {
		splx(s);
		so->so_pcb = NULL;
		free(rp);
		return error;
	}
	RTSOCK_LOCK();
	switch(rp->rcb_proto.sp_protocol) {
	case AF_INET:
		route_cb.ip_count++;
		break;
	case AF_INET6:
		route_cb.ip6_count++;
		break;
	case AF_IPX:
		route_cb.ipx_count++;
		break;
	}
	route_cb.any_count++;
	soisconnected(so);
	RTSOCK_UNLOCK();
	so->so_options |= SO_USELOOPBACK;
	splx(s);
	return 0;
}

static int
rts_bind(struct socket *so, struct bsd_sockaddr *nam, struct thread *td)
{

	return (raw_usrreqs.pru_bind(so, nam, td)); /* xxx just EINVAL */
}

static int
rts_connect(struct socket *so, struct bsd_sockaddr *nam, struct thread *td)
{

	return (raw_usrreqs.pru_connect(so, nam, td)); /* XXX just EINVAL */
}

/* pru_connect2 is EOPNOTSUPP */
/* pru_control is EOPNOTSUPP */

static void
rts_detach(struct socket *so)
{
	struct rawcb *rp = sotorawcb(so);

	KASSERT(rp != NULL, ("rts_detach: rp == NULL"));

	RTSOCK_LOCK();
	switch(rp->rcb_proto.sp_protocol) {
	case AF_INET:
		route_cb.ip_count--;
		break;
	case AF_INET6:
		route_cb.ip6_count--;
		break;
	case AF_IPX:
		route_cb.ipx_count--;
		break;
	}
	route_cb.any_count--;
	RTSOCK_UNLOCK();
	raw_usrreqs.pru_detach(so);
}

static int
rts_disconnect(struct socket *so)
{

	return (raw_usrreqs.pru_disconnect(so));
}

/* pru_listen is EOPNOTSUPP */

static int
rts_peeraddr(struct socket *so, struct bsd_sockaddr **nam)
{

	return (raw_usrreqs.pru_peeraddr(so, nam));
}

/* pru_rcvd is EOPNOTSUPP */
/* pru_rcvoob is EOPNOTSUPP */

static int
rts_send(struct socket *so, int flags, struct mbuf *m, struct bsd_sockaddr *nam,
	 struct mbuf *control, struct thread *td)
{

	return (raw_usrreqs.pru_send(so, flags, m, nam, control, td));
}

/* pru_sense is null */

static int
rts_shutdown(struct socket *so)
{

	return (raw_usrreqs.pru_shutdown(so));
}

static int
rts_sockaddr(struct socket *so, struct bsd_sockaddr **nam)
{

	return (raw_usrreqs.pru_sockaddr(so, nam));
}

static struct pr_usrreqs route_usrreqs = initialize_with([] (pr_usrreqs& x) {
	x.pru_abort =		rts_abort;
	x.pru_attach =		rts_attach;
	x.pru_bind =		rts_bind;
	x.pru_connect =		rts_connect;
	x.pru_detach =		rts_detach;
	x.pru_disconnect =	rts_disconnect;
	x.pru_peeraddr =		rts_peeraddr;
	x.pru_send =		rts_send;
	x.pru_shutdown =		rts_shutdown;
	x.pru_sockaddr =		rts_sockaddr;
	x.pru_close =		rts_close;
});

/*ARGSUSED*/
static int
route_output(struct mbuf *m, struct socket *so)
{
#define	sa_equal(a1, a2) (bcmp((a1), (a2), (a1)->sa_len) == 0)
	struct rt_msghdr *rtm = NULL;
	struct rtentry *rt = NULL;
	struct radix_node_head *rnh;
	struct rt_addrinfo info;
	int len, error = 0;
	struct ifnet *ifp = NULL;
	bsd_sa_family_t saf = AF_UNSPEC;

#define senderr(e) { error = e; goto flush;}
	if (m == NULL || ((m->m_hdr.mh_len < sizeof(long)) &&
		       (m = m_pullup(m, sizeof(long))) == NULL))
		return (ENOBUFS);
	if ((m->m_hdr.mh_flags & M_PKTHDR) == 0)
		panic("route_output");
	len = m->M_dat.MH.MH_pkthdr.len;
	if (len < sizeof(*rtm) ||
	    len != mtod(m, struct rt_msghdr *)->rtm_msglen) {
		info.rti_info[RTAX_DST] = NULL;
		senderr(EINVAL);
	}
	R_Malloc(rtm, struct rt_msghdr *, len);
	if (rtm == NULL) {
		info.rti_info[RTAX_DST] = NULL;
		senderr(ENOBUFS);
	}
	m_copydata(m, 0, len, (caddr_t)rtm);
	if (rtm->rtm_version != RTM_VERSION) {
		info.rti_info[RTAX_DST] = NULL;
		senderr(EPROTONOSUPPORT);
	}
	rtm->rtm_pid = osv_curtid();
	bzero(&info, sizeof(info));
	info.rti_addrs = rtm->rtm_addrs;
	if (rt_xaddrs((caddr_t)(rtm + 1), len + (caddr_t)rtm, &info)) {
		info.rti_info[RTAX_DST] = NULL;
		senderr(EINVAL);
	}
	info.rti_flags = rtm->rtm_flags;
	if (info.rti_info[RTAX_DST] == NULL ||
	    info.rti_info[RTAX_DST]->sa_family >= AF_MAX ||
	    (info.rti_info[RTAX_GATEWAY] != NULL &&
	     info.rti_info[RTAX_GATEWAY]->sa_family >= AF_MAX))
		senderr(EINVAL);
	saf = info.rti_info[RTAX_DST]->sa_family;
	/*
	 * Verify that the caller has the appropriate privilege; RTM_GET
	 * is the only operation the non-superuser is allowed.
	 */
	if (rtm->rtm_type != RTM_GET) {
		error = priv_check(curthread, PRIV_NET_ROUTE);
		if (error)
			senderr(error);
	}

	/*
	 * The given gateway address may be an interface address.
	 * For example, issuing a "route change" command on a route
	 * entry that was created from a tunnel, and the gateway
	 * address given is the local end point. In this case the 
	 * RTF_GATEWAY flag must be cleared or the destination will
	 * not be reachable even though there is no error message.
	 */
	if (info.rti_info[RTAX_GATEWAY] != NULL &&
	    info.rti_info[RTAX_GATEWAY]->sa_family != AF_LINK) {
		struct route gw_ro;

		bzero(&gw_ro, sizeof(gw_ro));
		gw_ro.ro_dst = *info.rti_info[RTAX_GATEWAY];
		rtalloc_ign_fib(&gw_ro, 0, so->so_fibnum);
		/* 
		 * A host route through the loopback interface is 
		 * installed for each interface adddress. In pre 8.0
		 * releases the interface address of a PPP link type
		 * is not reachable locally. This behavior is fixed as 
		 * part of the new L2/L3 redesign and rewrite work. The
		 * signature of this interface address route is the
		 * AF_LINK sa_family type of the rt_gateway, and the
		 * rt_ifp has the IFF_LOOPBACK flag set.
		 */
		if (gw_ro.ro_rt != NULL &&
		    gw_ro.ro_rt->rt_gateway->sa_family == AF_LINK &&
		    gw_ro.ro_rt->rt_ifp->if_flags & IFF_LOOPBACK)
			info.rti_flags &= ~RTF_GATEWAY;
		if (gw_ro.ro_rt != NULL)
			RTFREE(gw_ro.ro_rt);
	}

	switch (rtm->rtm_type) {
		struct rtentry *saved_nrt;

	case RTM_ADD:
		if (info.rti_info[RTAX_GATEWAY] == NULL)
			senderr(EINVAL);
		saved_nrt = NULL;

		/* support for new ARP code */
		if (info.rti_info[RTAX_GATEWAY]->sa_family == AF_LINK &&
		    (rtm->rtm_flags & RTF_LLDATA) != 0) {
			error = lla_rt_output(rtm, &info);
			break;
		}
		error = rtrequest1_fib(RTM_ADD, &info, &saved_nrt,
		    so->so_fibnum);
		if (error == 0 && saved_nrt) {
			RT_LOCK(saved_nrt);
			rt_setmetrics(rtm->rtm_inits,
				&rtm->rtm_rmx, &saved_nrt->rt_rmx);
			rtm->rtm_index = saved_nrt->rt_ifp->if_index;
			RT_REMREF(saved_nrt);
			RT_UNLOCK(saved_nrt);
		}
		break;

	case RTM_DELETE:
		saved_nrt = NULL;
		/* support for new ARP code */
		if (info.rti_info[RTAX_GATEWAY] && 
		    (info.rti_info[RTAX_GATEWAY]->sa_family == AF_LINK) &&
		    (rtm->rtm_flags & RTF_LLDATA) != 0) {
			error = lla_rt_output(rtm, &info);
			break;
		}
		error = rtrequest1_fib(RTM_DELETE, &info, &saved_nrt,
		    so->so_fibnum);
		if (error == 0) {
			RT_LOCK(saved_nrt);
			rt = saved_nrt;
			goto report;
		}
		break;

	case RTM_GET:
	case RTM_CHANGE:
	case RTM_LOCK:
		rnh = rt_tables_get_rnh(so->so_fibnum,
		    info.rti_info[RTAX_DST]->sa_family);
		if (rnh == NULL)
			senderr(EAFNOSUPPORT);
		RADIX_NODE_HEAD_RLOCK(rnh);
		rt = (struct rtentry *) rnh->rnh_lookup(info.rti_info[RTAX_DST],
			info.rti_info[RTAX_NETMASK], rnh);
		if (rt == NULL) {	/* XXX looks bogus */
			RADIX_NODE_HEAD_RUNLOCK(rnh);
			senderr(ESRCH);
		}
#ifdef RADIX_MPATH
		/*
		 * for RTM_CHANGE/LOCK, if we got multipath routes,
		 * we require users to specify a matching RTAX_GATEWAY.
		 *
		 * for RTM_GET, gate is optional even with multipath.
		 * if gate == NULL the first match is returned.
		 * (no need to call rt_mpath_matchgate if gate == NULL)
		 */
		if (rn_mpath_capable(rnh) &&
		    (rtm->rtm_type != RTM_GET || info.rti_info[RTAX_GATEWAY])) {
			rt = rt_mpath_matchgate(rt, info.rti_info[RTAX_GATEWAY]);
			if (!rt) {
				RADIX_NODE_HEAD_RUNLOCK(rnh);
				senderr(ESRCH);
			}
		}
#endif
		/*
		 * If performing proxied L2 entry insertion, and
		 * the actual PPP host entry is found, perform
		 * another search to retrieve the prefix route of
		 * the local end point of the PPP link.
		 */
		if (rtm->rtm_flags & RTF_ANNOUNCE) {
			struct bsd_sockaddr laddr;

			if (rt->rt_ifp != NULL && 
			    rt->rt_ifp->if_type == IFT_PROPVIRTUAL) {
				struct bsd_ifaddr *ifa;

				ifa = ifa_ifwithnet(info.rti_info[RTAX_DST], 1);
				if (ifa != NULL)
					rt_maskedcopy(ifa->ifa_addr,
						      &laddr,
						      ifa->ifa_netmask);
			} else
				rt_maskedcopy(rt->rt_ifa->ifa_addr,
					      &laddr,
					      rt->rt_ifa->ifa_netmask);
			/* 
			 * refactor rt and no lock operation necessary
			 */
			rt = (struct rtentry *)rnh->rnh_matchaddr(&laddr, rnh);
			if (rt == NULL) {
				RADIX_NODE_HEAD_RUNLOCK(rnh);
				senderr(ESRCH);
			}
		} 
		RT_LOCK(rt);
		RT_ADDREF(rt);
		RADIX_NODE_HEAD_RUNLOCK(rnh);

		/* 
		 * Fix for PR: 82974
		 *
		 * RTM_CHANGE/LOCK need a perfect match, rn_lookup()
		 * returns a perfect match in case a netmask is
		 * specified.  For host routes only a longest prefix
		 * match is returned so it is necessary to compare the
		 * existence of the netmask.  If both have a netmask
		 * rnh_lookup() did a perfect match and if none of them
		 * have a netmask both are host routes which is also a
		 * perfect match.
		 */

		if (rtm->rtm_type != RTM_GET && 
		    (!rt_mask(rt) != !info.rti_info[RTAX_NETMASK])) {
			RT_UNLOCK(rt);
			senderr(ESRCH);
		}

		switch(rtm->rtm_type) {

		case RTM_GET:
		report:
			RT_LOCK_ASSERT(rt);
			info.rti_info[RTAX_DST] = rt_key(rt);
			info.rti_info[RTAX_GATEWAY] = rt->rt_gateway;
			info.rti_info[RTAX_NETMASK] = rt_mask(rt);
			info.rti_info[RTAX_GENMASK] = 0;
			if (rtm->rtm_addrs & (RTA_IFP | RTA_IFA)) {
				ifp = rt->rt_ifp;
				if (ifp) {
					info.rti_info[RTAX_IFP] =
					    ifp->if_addr->ifa_addr;
					if (ifp->if_flags & IFF_POINTOPOINT)
						info.rti_info[RTAX_BRD] =
						    rt->rt_ifa->ifa_dstaddr;
					rtm->rtm_index = ifp->if_index;
				} else {
					info.rti_info[RTAX_IFP] = NULL;
					info.rti_info[RTAX_IFA] = NULL;
				}
			} else if ((ifp = rt->rt_ifp) != NULL) {
				rtm->rtm_index = ifp->if_index;
			}
			len = rt_msg2(rtm->rtm_type, &info, NULL, NULL);
			if (len > rtm->rtm_msglen) {
				struct rt_msghdr *new_rtm;
				R_Malloc(new_rtm, struct rt_msghdr *, len);
				if (new_rtm == NULL) {
					RT_UNLOCK(rt);
					senderr(ENOBUFS);
				}
				bcopy(rtm, new_rtm, rtm->rtm_msglen);
				Free(rtm); rtm = new_rtm;
			}
			(void)rt_msg2(rtm->rtm_type, &info, (caddr_t)rtm, NULL);
			rtm->rtm_flags = rt->rt_flags;
			rt_getmetrics(&rt->rt_rmx, &rtm->rtm_rmx);
			rtm->rtm_addrs = info.rti_addrs;
			break;

		case RTM_CHANGE:
			/*
			 * New gateway could require new bsd_ifaddr, ifp;
			 * flags may also be different; ifp may be specified
			 * by ll bsd_sockaddr when protocol address is ambiguous
			 */
			if (((rt->rt_flags & RTF_GATEWAY) &&
			     info.rti_info[RTAX_GATEWAY] != NULL) ||
			    info.rti_info[RTAX_IFP] != NULL ||
			    (info.rti_info[RTAX_IFA] != NULL &&
			     !sa_equal(info.rti_info[RTAX_IFA],
				       rt->rt_ifa->ifa_addr))) {
				RT_UNLOCK(rt);
				RADIX_NODE_HEAD_LOCK(rnh);
				error = rt_getifa_fib(&info, rt->rt_fibnum);
				/*
				 * XXXRW: Really we should release this
				 * reference later, but this maintains
				 * historical behavior.
				 */
				if (info.rti_ifa != NULL)
					ifa_free(info.rti_ifa);
				RADIX_NODE_HEAD_UNLOCK(rnh);
				if (error != 0)
					senderr(error);
				RT_LOCK(rt);
			}
			if (info.rti_ifa != NULL &&
			    info.rti_ifa != rt->rt_ifa &&
			    rt->rt_ifa != NULL &&
			    rt->rt_ifa->ifa_rtrequest != NULL) {
				rt->rt_ifa->ifa_rtrequest(RTM_DELETE, rt,
				    &info);
				ifa_free(rt->rt_ifa);
			}
			if (info.rti_info[RTAX_GATEWAY] != NULL) {
				RT_UNLOCK(rt);
				RADIX_NODE_HEAD_LOCK(rnh);
				RT_LOCK(rt);
				
				error = rt_setgate(rt, rt_key(rt),
				    info.rti_info[RTAX_GATEWAY]);
				RADIX_NODE_HEAD_UNLOCK(rnh);
				if (error != 0) {
					RT_UNLOCK(rt);
					senderr(error);
				}
				rt->rt_flags |= (RTF_GATEWAY & info.rti_flags);
			}
			if (info.rti_ifa != NULL &&
			    info.rti_ifa != rt->rt_ifa) {
				ifa_ref(info.rti_ifa);
				rt->rt_ifa = info.rti_ifa;
				rt->rt_ifp = info.rti_ifp;
			}
			/* Allow some flags to be toggled on change. */
			rt->rt_flags = (rt->rt_flags & ~RTF_FMASK) |
				    (rtm->rtm_flags & RTF_FMASK);
			rt_setmetrics(rtm->rtm_inits, &rtm->rtm_rmx,
					&rt->rt_rmx);
			rtm->rtm_index = rt->rt_ifp->if_index;
			if (rt->rt_ifa && rt->rt_ifa->ifa_rtrequest)
			       rt->rt_ifa->ifa_rtrequest(RTM_ADD, rt, &info);
			/* FALLTHROUGH */
		case RTM_LOCK:
			/* We don't support locks anymore */
			break;
		}
		RT_UNLOCK(rt);
		break;

	default:
		senderr(EOPNOTSUPP);
	}

flush:
	if (rtm) {
		if (error)
			rtm->rtm_errno = error;
		else
			rtm->rtm_flags |= RTF_DONE;
	}
	if (rt)		/* XXX can this be true? */
		RTFREE(rt);
    {
	struct rawcb *rp = NULL;
	/*
	 * Check to see if we don't want our own messages.
	 */
	if ((so->so_options & SO_USELOOPBACK) == 0) {
		if (route_cb.any_count <= 1) {
			if (rtm)
				Free(rtm);
			m_freem(m);
			return (error);
		}
		/* There is another listener, so construct message */
		rp = sotorawcb(so);
	}
	if (rtm) {
		m_copyback(m, 0, rtm->rtm_msglen, (caddr_t)rtm);
		if (m->M_dat.MH.MH_pkthdr.len < rtm->rtm_msglen) {
			m_freem(m);
			m = NULL;
		} else if (m->M_dat.MH.MH_pkthdr.len > rtm->rtm_msglen)
			m_adj(m, rtm->rtm_msglen - m->M_dat.MH.MH_pkthdr.len);
	}
	if (m) {
		M_SETFIB(m, so->so_fibnum);
		m->m_hdr.mh_flags |= RTS_FILTER_FIB;
		if (rp) {
			/*
			 * XXX insure we don't get a copy by
			 * invalidating our protocol
			 */
			unsigned short family = rp->rcb_proto.sp_family;
			rp->rcb_proto.sp_family = 0;
			rt_dispatch(m, saf);
			rp->rcb_proto.sp_family = family;
		} else
			rt_dispatch(m, saf);
	}
	/* info.rti_info[RTAX_DST] (used above) can point inside of rtm */
	if (rtm)
		Free(rtm);
    }
	return (error);
#undef	sa_equal
}

static void
rt_setmetrics(u_long which, const struct rt_metrics *in,
	struct rt_metrics_lite *out)
{
    struct timeval tv;
    getmicrotime(&tv);
#define metric(f, e) if (which & (f)) out->e = in->e;
	/*
	 * Only these are stored in the routing entry since introduction
	 * of tcp hostcache. The rest is ignored.
	 */
	metric(RTV_MTU, rmx_mtu);
	metric(RTV_WEIGHT, rmx_weight);
	/* Userland -> kernel timebase conversion. */
	if (which & RTV_EXPIRE)
		out->rmx_expire = in->rmx_expire ?
		    in->rmx_expire - tv.tv_sec + time_uptime : 0;
#undef metric
}

static void
rt_getmetrics(const struct rt_metrics_lite *in, struct rt_metrics *out)
{
    struct timeval tv;
    getmicrotime(&tv);
#define metric(e) out->e = in->e;
	bzero(out, sizeof(*out));
	metric(rmx_mtu);
	metric(rmx_weight);
	/* Kernel -> userland timebase conversion. */
	out->rmx_expire = in->rmx_expire ?
	    in->rmx_expire - time_uptime + tv.tv_sec : 0;
#undef metric
}

/*
 * Extract the addresses of the passed bsd_sockaddrs.
 * Do a little sanity checking so as to avoid bad memory references.
 * This data is derived straight from userland.
 */
static int
rt_xaddrs(caddr_t cp, caddr_t cplim, struct rt_addrinfo *rtinfo)
{
	struct bsd_sockaddr *sa;
	int i;

	for (i = 0; i < RTAX_MAX && cp < cplim; i++) {
		if ((rtinfo->rti_addrs & (1 << i)) == 0)
			continue;
		sa = (struct bsd_sockaddr *)cp;
		/*
		 * It won't fit.
		 */
		if (cp + sa->sa_len > cplim)
			return (EINVAL);
		/*
		 * there are no more.. quit now
		 * If there are more bits, they are in error.
		 * I've seen this. route(1) can evidently generate these. 
		 * This causes kernel to core dump.
		 * for compatibility, If we see this, point to a safe address.
		 */
		if (sa->sa_len == 0) {
			rtinfo->rti_info[i] = &sa_zero;
			return (0); /* should be EINVAL but for compat */
		}
		/* accept it */
		rtinfo->rti_info[i] = sa;
		cp += SA_SIZE(sa);
	}
	return (0);
}

/*
 * Used by the routing socket.
 */
static struct mbuf *
rt_msg1(int type, struct rt_addrinfo *rtinfo)
{
	struct rt_msghdr *rtm;
	struct mbuf *m;
	int i;
	struct bsd_sockaddr *sa;
	int len, dlen;

	switch (type) {

	case RTM_DELADDR:
	case RTM_NEWADDR:
		len = sizeof(struct ifa_msghdr);
		break;

	case RTM_DELMADDR:
	case RTM_NEWMADDR:
		len = sizeof(struct ifma_msghdr);
		break;

	case RTM_IFINFO:
		len = sizeof(struct if_msghdr);
		break;

	case RTM_IFANNOUNCE:
	case RTM_IEEE80211:
		len = sizeof(struct if_announcemsghdr);
		break;

	default:
		len = sizeof(struct rt_msghdr);
	}
	if (len > MCLBYTES)
		panic("rt_msg1");
	m = m_gethdr(M_DONTWAIT, MT_DATA);
	if (m && len > MHLEN) {
		MCLGET(m, M_DONTWAIT);
		if ((m->m_hdr.mh_flags & M_EXT) == 0) {
			m_free(m);
			m = NULL;
		}
	}
	if (m == NULL)
		return (m);
	m->M_dat.MH.MH_pkthdr.len = m->m_hdr.mh_len = len;
	m->M_dat.MH.MH_pkthdr.rcvif = NULL;
	rtm = mtod(m, struct rt_msghdr *);
	bzero((caddr_t)rtm, len);
	for (i = 0; i < RTAX_MAX; i++) {
		if ((sa = rtinfo->rti_info[i]) == NULL)
			continue;
		rtinfo->rti_addrs |= (1 << i);
		dlen = SA_SIZE(sa);
		m_copyback(m, len, dlen, (caddr_t)sa);
		len += dlen;
	}
	if (m->M_dat.MH.MH_pkthdr.len != len) {
		m_freem(m);
		return (NULL);
	}
	rtm->rtm_msglen = len;
	rtm->rtm_version = RTM_VERSION;
	rtm->rtm_type = type;
	return (m);
}

/*
 * Used by the sysctl code and routing socket.
 */
static int
rt_msg2(int type, struct rt_addrinfo *rtinfo, caddr_t cp, struct walkarg *w)
{
	int i;
	int len, dlen, second_time = 0;
	caddr_t cp0;

	rtinfo->rti_addrs = 0;
again:
	switch (type) {

	case RTM_DELADDR:
	case RTM_NEWADDR:
		if (w != NULL && w->w_op == NET_RT_IFLISTL) {
#ifdef COMPAT_FREEBSD32
			if (w->w_req->flags & SCTL_MASK32)
				len = sizeof(struct ifa_msghdrl32);
			else
#endif
				len = sizeof(struct ifa_msghdrl);
		} else
			len = sizeof(struct ifa_msghdr);
		break;

	case RTM_IFINFO:
#ifdef COMPAT_FREEBSD32
		if (w != NULL && w->w_req->flags & SCTL_MASK32) {
			if (w->w_op == NET_RT_IFLISTL)
				len = sizeof(struct if_msghdrl32);
			else
				len = sizeof(struct if_msghdr32);
			break;
		}
#endif
		if (w != NULL && w->w_op == NET_RT_IFLISTL)
			len = sizeof(struct if_msghdrl);
		else
			len = sizeof(struct if_msghdr);
		break;

	case RTM_NEWMADDR:
		len = sizeof(struct ifma_msghdr);
		break;

	default:
		len = sizeof(struct rt_msghdr);
	}
	cp0 = cp;
	if (cp0)
		cp += len;
	for (i = 0; i < RTAX_MAX; i++) {
		struct bsd_sockaddr *sa;

		if ((sa = rtinfo->rti_info[i]) == NULL)
			continue;
		rtinfo->rti_addrs |= (1 << i);
		dlen = SA_SIZE(sa);
		if (cp) {
			bcopy((caddr_t)sa, cp, (unsigned)dlen);
			cp += dlen;
		}
		len += dlen;
	}
	len = ALIGN(len);
	if (cp == NULL && w != NULL && !second_time) {
		struct walkarg *rw = w;

		if (rw->w_req) {
			if (rw->w_tmemsize < len) {
				if (rw->w_tmem)
					free(rw->w_tmem);
				rw->w_tmem = (caddr_t)malloc(len);
				if (rw->w_tmem)
					rw->w_tmemsize = len;
			}
			if (rw->w_tmem) {
				cp = rw->w_tmem;
				second_time = 1;
				goto again;
			}
		}
	}
	if (cp) {
		struct rt_msghdr *rtm = (struct rt_msghdr *)cp0;

		rtm->rtm_version = RTM_VERSION;
		rtm->rtm_type = type;
		rtm->rtm_msglen = len;
	}
	return (len);
}

/*
 * This routine is called to generate a message from the routing
 * socket indicating that a redirect has occured, a routing lookup
 * has failed, or that a protocol has detected timeouts to a particular
 * destination.
 */
void
rt_missmsg_fib(int type, struct rt_addrinfo *rtinfo, int flags, int error,
    int fibnum)
{
	struct rt_msghdr *rtm;
	struct mbuf *m;
	struct bsd_sockaddr *sa = rtinfo->rti_info[RTAX_DST];

	if (route_cb.any_count == 0)
		return;
	m = rt_msg1(type, rtinfo);
	if (m == NULL)
		return;

	if (fibnum != RTS_ALLFIBS) {
		KASSERT(fibnum >= 0 && fibnum < rt_numfibs, ("%s: fibnum out "
		    "of range 0 <= %d < %d", __func__, fibnum, rt_numfibs));
		M_SETFIB(m, fibnum);
		m->m_hdr.mh_flags |= RTS_FILTER_FIB;
	}

	rtm = mtod(m, struct rt_msghdr *);
	rtm->rtm_flags = RTF_DONE | flags;
	rtm->rtm_errno = error;
	rtm->rtm_addrs = rtinfo->rti_addrs;
	rt_dispatch(m, sa ? sa->sa_family : AF_UNSPEC);
}

void
rt_missmsg(int type, struct rt_addrinfo *rtinfo, int flags, int error)
{

	rt_missmsg_fib(type, rtinfo, flags, error, RTS_ALLFIBS);
}

/*
 * This routine is called to generate a message from the routing
 * socket indicating that the status of a network interface has changed.
 */
void
rt_ifmsg(struct ifnet *ifp)
{
	struct if_msghdr *ifm;
	struct mbuf *m;
	struct rt_addrinfo info;

	if (route_cb.any_count == 0)
		return;
	bzero((caddr_t)&info, sizeof(info));
	m = rt_msg1(RTM_IFINFO, &info);
	if (m == NULL)
		return;
	ifm = mtod(m, struct if_msghdr *);
	ifm->ifm_index = ifp->if_index;
	ifm->ifm_flags = ifp->if_flags | ifp->if_drv_flags;
	ifm->ifm_data = ifp->if_data;
	ifm->ifm_addrs = 0;
	rt_dispatch(m, AF_UNSPEC);
}

/*
 * This is called to generate messages from the routing socket
 * indicating a network interface has had addresses associated with it.
 * if we ever reverse the logic and replace messages TO the routing
 * socket indicate a request to configure interfaces, then it will
 * be unnecessary as the routing socket will automatically generate
 * copies of it.
 */
void
rt_newaddrmsg_fib(int cmd, struct bsd_ifaddr *ifa, int error, struct rtentry *rt,
    int fibnum)
{
	struct rt_addrinfo info;
	struct bsd_sockaddr *sa = NULL;
	int pass;
	struct mbuf *m = NULL;
	struct ifnet *ifp = ifa->ifa_ifp;

	KASSERT(cmd == RTM_ADD || cmd == RTM_DELETE,
		("unexpected cmd %u", cmd));
#if defined(INET) || defined(INET6)
#ifdef SCTP
	/*
	 * notify the SCTP stack
	 * this will only get called when an address is added/deleted
	 * XXX pass the bsd_ifaddr struct instead if ifa->ifa_addr...
	 */
	sctp_addr_change(ifa, cmd);
#endif /* SCTP */
#endif
	if (route_cb.any_count == 0)
		return;
	for (pass = 1; pass < 3; pass++) {
		bzero((caddr_t)&info, sizeof(info));
		if ((cmd == RTM_ADD && pass == 1) ||
		    (cmd == RTM_DELETE && pass == 2)) {
			struct ifa_msghdr *ifam;
			int ncmd = cmd == RTM_ADD ? RTM_NEWADDR : RTM_DELADDR;

			info.rti_info[RTAX_IFA] = sa = ifa->ifa_addr;
			info.rti_info[RTAX_IFP] = ifp->if_addr->ifa_addr;
			info.rti_info[RTAX_NETMASK] = ifa->ifa_netmask;
			info.rti_info[RTAX_BRD] = ifa->ifa_dstaddr;
			if ((m = rt_msg1(ncmd, &info)) == NULL)
				continue;
			ifam = mtod(m, struct ifa_msghdr *);
			ifam->ifam_index = ifp->if_index;
			ifam->ifam_metric = ifa->ifa_metric;
			ifam->ifam_flags = ifa->ifa_flags;
			ifam->ifam_addrs = info.rti_addrs;
		}
		if ((cmd == RTM_ADD && pass == 2) ||
		    (cmd == RTM_DELETE && pass == 1)) {
			struct rt_msghdr *rtm;

			if (rt == NULL)
				continue;
			info.rti_info[RTAX_NETMASK] = rt_mask(rt);
			info.rti_info[RTAX_DST] = sa = rt_key(rt);
			info.rti_info[RTAX_GATEWAY] = rt->rt_gateway;
			if ((m = rt_msg1(cmd, &info)) == NULL)
				continue;
			rtm = mtod(m, struct rt_msghdr *);
			rtm->rtm_index = ifp->if_index;
			rtm->rtm_flags |= rt->rt_flags;
			rtm->rtm_errno = error;
			rtm->rtm_addrs = info.rti_addrs;
		}
		if (fibnum != RTS_ALLFIBS) {
			KASSERT(fibnum >= 0 && fibnum < rt_numfibs, ("%s: "
			    "fibnum out of range 0 <= %d < %d", __func__,
			     fibnum, rt_numfibs));
			M_SETFIB(m, fibnum);
			m->m_hdr.mh_flags |= RTS_FILTER_FIB;
		}
		rt_dispatch(m, sa ? sa->sa_family : AF_UNSPEC);
	}
}

void
rt_newaddrmsg(int cmd, struct bsd_ifaddr *ifa, int error, struct rtentry *rt)
{

	rt_newaddrmsg_fib(cmd, ifa, error, rt, RTS_ALLFIBS);
}

/*
 * This is the analogue to the rt_newaddrmsg which performs the same
 * function but for multicast group memberhips.  This is easier since
 * there is no route state to worry about.
 */
void
rt_newmaddrmsg(int cmd, struct ifmultiaddr *ifma)
{
	struct rt_addrinfo info;
	struct mbuf *m = NULL;
	struct ifnet *ifp = ifma->ifma_ifp;
	struct ifma_msghdr *ifmam;

	if (route_cb.any_count == 0)
		return;

	bzero((caddr_t)&info, sizeof(info));
	info.rti_info[RTAX_IFA] = ifma->ifma_addr;
	info.rti_info[RTAX_IFP] = ifp ? ifp->if_addr->ifa_addr : NULL;
	/*
	 * If a link-layer address is present, present it as a ``gateway''
	 * (similarly to how ARP entries, e.g., are presented).
	 */
	info.rti_info[RTAX_GATEWAY] = ifma->ifma_lladdr;
	m = rt_msg1(cmd, &info);
	if (m == NULL)
		return;
	ifmam = mtod(m, struct ifma_msghdr *);
	KASSERT(ifp != NULL, ("%s: link-layer multicast address w/o ifp\n",
	    __func__));
	ifmam->ifmam_index = ifp->if_index;
	ifmam->ifmam_addrs = info.rti_addrs;
	rt_dispatch(m, ifma->ifma_addr ? ifma->ifma_addr->sa_family : AF_UNSPEC);
}

static struct mbuf *
rt_makeifannouncemsg(struct ifnet *ifp, int type, int what,
	struct rt_addrinfo *info)
{
	struct if_announcemsghdr *ifan;
	struct mbuf *m;

	if (route_cb.any_count == 0)
		return NULL;
	bzero((caddr_t)info, sizeof(*info));
	m = rt_msg1(type, info);
	if (m != NULL) {
		ifan = mtod(m, struct if_announcemsghdr *);
		ifan->ifan_index = ifp->if_index;
		strlcpy(ifan->ifan_name, ifp->if_xname,
			sizeof(ifan->ifan_name));
		ifan->ifan_what = what;
	}
	return m;
}

/*
 * This is called to generate routing socket messages indicating
 * IEEE80211 wireless events.
 * XXX we piggyback on the RTM_IFANNOUNCE msg format in a clumsy way.
 */
void
rt_ieee80211msg(struct ifnet *ifp, int what, void *data, size_t data_len)
{
	struct mbuf *m;
	struct rt_addrinfo info;

	m = rt_makeifannouncemsg(ifp, RTM_IEEE80211, what, &info);
	if (m != NULL) {
		/*
		 * Append the ieee80211 data.  Try to stick it in the
		 * mbuf containing the ifannounce msg; otherwise allocate
		 * a new mbuf and append.
		 *
		 * NB: we assume m is a single mbuf.
		 */
		if (data_len > M_TRAILINGSPACE(m)) {
			struct mbuf *n = m_get(M_NOWAIT, MT_DATA);
			if (n == NULL) {
				m_freem(m);
				return;
			}
			bcopy(data, mtod(n, void *), data_len);
			n->m_hdr.mh_len = data_len;
			m->m_hdr.mh_next = n;
		} else if (data_len > 0) {
			bcopy(data, mtod(m, u_int8_t *) + m->m_hdr.mh_len, data_len);
			m->m_hdr.mh_len += data_len;
		}
		if (m->m_hdr.mh_flags & M_PKTHDR)
			m->M_dat.MH.MH_pkthdr.len += data_len;
		mtod(m, struct if_announcemsghdr *)->ifan_msglen += data_len;
		rt_dispatch(m, AF_UNSPEC);
	}
}

/*
 * This is called to generate routing socket messages indicating
 * network interface arrival and departure.
 */
void
rt_ifannouncemsg(struct ifnet *ifp, int what)
{
	struct mbuf *m;
	struct rt_addrinfo info;

	m = rt_makeifannouncemsg(ifp, RTM_IFANNOUNCE, what, &info);
	if (m != NULL)
		rt_dispatch(m, AF_UNSPEC);
}

static void
rt_dispatch(struct mbuf *m, bsd_sa_family_t saf)
{
	struct m_tag *tag;

	/*
	 * Preserve the family from the bsd_sockaddr, if any, in an m_tag for
	 * use when injecting the mbuf into the routing socket buffer from
	 * the netisr.
	 */
	if (saf != AF_UNSPEC) {
		tag = m_tag_get(PACKET_TAG_RTSOCKFAM, sizeof(unsigned short),
		    M_NOWAIT);
		if (tag == NULL) {
			m_freem(m);
			return;
		}
		*(unsigned short *)(tag + 1) = saf;
		m_tag_prepend(m, tag);
	}

	netisr_queue(NETISR_ROUTE, m);	/* mbuf is free'd on failure. */
}

/*
 * This is used in dumping the kernel table via sysctl().
 */
int
sysctl_dumpentry(struct radix_node *rn, void *vw)
{
	struct walkarg *w = (walkarg *)vw;
	struct rtentry *rt = (struct rtentry *)rn;
	int error = 0, size;
	struct rt_addrinfo info;

#if 0    
	if (w->w_op == NET_RT_FLAGS && !(rt->rt_flags & w->w_arg))
		return 0;
    
	if ((rt->rt_flags & RTF_HOST) == 0
	    ? jailed_without_vnet(w->w_req->td->td_ucred)
	    : prison_if(w->w_req->td->td_ucred, rt_key(rt)) != 0)
		return (0);
#endif    
	bzero((caddr_t)&info, sizeof(info));
	info.rti_info[RTAX_DST] = rt_key(rt);
	info.rti_info[RTAX_GATEWAY] = rt->rt_gateway;
	info.rti_info[RTAX_NETMASK] = rt_mask(rt);
	info.rti_info[RTAX_GENMASK] = 0;
	if (rt->rt_ifp) {
		info.rti_info[RTAX_IFP] = rt->rt_ifp->if_addr->ifa_addr;
		info.rti_info[RTAX_IFA] = rt->rt_ifa->ifa_addr;
		if (rt->rt_ifp->if_flags & IFF_POINTOPOINT)
			info.rti_info[RTAX_BRD] = rt->rt_ifa->ifa_dstaddr;
	}
	size = rt_msg2(RTM_GET, &info, NULL, w);
	if (w->w_req && w->w_tmem) {
		struct rt_msghdr *rtm = (struct rt_msghdr *)w->w_tmem;

		rtm->rtm_flags = rt->rt_flags;
		/*
		 * let's be honest about this being a retarded hack
		 */
		rtm->rtm_fmask = rt->rt_rmx.rmx_pksent;
		rt_getmetrics(&rt->rt_rmx, &rtm->rtm_rmx);
		rtm->rtm_index = rt->rt_ifp->if_index;
		rtm->rtm_errno = rtm->rtm_pid = rtm->rtm_seq = 0;
		rtm->rtm_addrs = info.rti_addrs;
		error = SYSCTL_OUT(w->w_req, (caddr_t)rtm, size);
		return (error);
	}
	return (error);
}

#ifdef COMPAT_FREEBSD32
static void
copy_ifdata32(struct if_data *src, struct if_data32 *dst)
{

	bzero(dst, sizeof(*dst));
	CP(*src, *dst, ifi_type);
	CP(*src, *dst, ifi_physical);
	CP(*src, *dst, ifi_addrlen);
	CP(*src, *dst, ifi_hdrlen);
	CP(*src, *dst, ifi_link_state);
	dst->ifi_datalen = sizeof(struct if_data32);
	CP(*src, *dst, ifi_mtu);
	CP(*src, *dst, ifi_metric);
	CP(*src, *dst, ifi_baudrate);
	CP(*src, *dst, ifi_ipackets);
	CP(*src, *dst, ifi_ierrors);
	CP(*src, *dst, ifi_opackets);
	CP(*src, *dst, ifi_oerrors);
	CP(*src, *dst, ifi_collisions);
	CP(*src, *dst, ifi_ibytes);
	CP(*src, *dst, ifi_obytes);
	CP(*src, *dst, ifi_imcasts);
	CP(*src, *dst, ifi_omcasts);
	CP(*src, *dst, ifi_iqdrops);
	CP(*src, *dst, ifi_noproto);
	CP(*src, *dst, ifi_hwassist);
	CP(*src, *dst, ifi_epoch);
	TV_CP(*src, *dst, ifi_lastchange);
}
#endif

#if 0
static int
sysctl_iflist_ifml(struct ifnet *ifp, struct rt_addrinfo *info,
    struct walkarg *w, int len)
{
	struct if_msghdrl *ifm;

	ifm = (struct if_msghdrl *)w->w_tmem;
	ifm->ifm_addrs = info->rti_addrs;
	ifm->ifm_flags = ifp->if_flags | ifp->if_drv_flags;
	ifm->ifm_index = ifp->if_index;
	ifm->_ifm_spare1 = 0;
	ifm->ifm_len = sizeof(*ifm);
	ifm->ifm_data_off = offsetof(struct if_msghdrl, ifm_data);

	ifm->ifm_data = ifp->if_data;

	return (SYSCTL_OUT(w->w_req, (caddr_t)ifm, len));
}
#endif

static int
sysctl_iflist_ifm(struct ifnet *ifp, struct rt_addrinfo *info,
    struct walkarg *w, int len)
{
	struct if_msghdr *ifm;

#ifdef COMPAT_FREEBSD32
	if (w->w_req->flags & SCTL_MASK32) {
		struct if_msghdr32 *ifm32;

		ifm32 = (struct if_msghdr32 *)w->w_tmem;
		ifm32->ifm_addrs = info->rti_addrs;
		ifm32->ifm_flags = ifp->if_flags | ifp->if_drv_flags;
		ifm32->ifm_index = ifp->if_index;

		copy_ifdata32(&ifp->if_data, &ifm32->ifm_data);

		return (SYSCTL_OUT(w->w_req, (caddr_t)ifm32, len));
	}
#endif
	ifm = (struct if_msghdr *)w->w_tmem;
	ifm->ifm_addrs = info->rti_addrs;
	ifm->ifm_flags = ifp->if_flags | ifp->if_drv_flags;
	ifm->ifm_index = ifp->if_index;

	ifm->ifm_data = ifp->if_data;

	return (SYSCTL_OUT(w->w_req, (caddr_t)ifm, len));
}
#if 0
static int
sysctl_iflist_ifaml(struct bsd_ifaddr *ifa, struct rt_addrinfo *info,
    struct walkarg *w, int len)
{
	struct ifa_msghdrl *ifam;

	ifam = (struct ifa_msghdrl *)w->w_tmem;
	ifam->ifam_addrs = info->rti_addrs;
	ifam->ifam_flags = ifa->ifa_flags;
	ifam->ifam_index = ifa->ifa_ifp->if_index;
	ifam->_ifam_spare1 = 0;
	ifam->ifam_len = sizeof(*ifam);
	ifam->ifam_data_off = offsetof(struct ifa_msghdrl, ifam_data);
	ifam->ifam_metric = ifa->ifa_metric;

	ifam->ifam_data = ifa->if_data;

	return (SYSCTL_OUT(w->w_req, w->w_tmem, len));
}
#endif
static int
sysctl_iflist_ifam(struct bsd_ifaddr *ifa, struct rt_addrinfo *info,
    struct walkarg *w, int len)
{
	struct ifa_msghdr *ifam;

	ifam = (struct ifa_msghdr *)w->w_tmem;
	ifam->ifam_addrs = info->rti_addrs;
	ifam->ifam_flags = ifa->ifa_flags;
	ifam->ifam_index = ifa->ifa_ifp->if_index;
	ifam->ifam_metric = ifa->ifa_metric;

	return (SYSCTL_OUT(w->w_req, w->w_tmem, len));
}

static int
sysctl_iflist(int af, struct walkarg *w)
{
	struct ifnet *ifp;
	struct bsd_ifaddr *ifa;
	struct rt_addrinfo info;
	int len, error = 0;

	bzero((caddr_t)&info, sizeof(info));
	IFNET_RLOCK();
	TAILQ_FOREACH(ifp, &V_ifnet, if_link) {
		if (w->w_arg && w->w_arg != ifp->if_index)
			continue;
		IF_ADDR_RLOCK(ifp);
		ifa = ifp->if_addr;
		info.rti_info[RTAX_IFP] = ifa->ifa_addr;
		len = rt_msg2(RTM_IFINFO, &info, NULL, w);
		info.rti_info[RTAX_IFP] = NULL;
		if (w->w_req && w->w_tmem) {
#if 0            
			if (w->w_op == NET_RT_IFLISTL)
				error = sysctl_iflist_ifml(ifp, &info, w, len);
			else
#endif                
				error = sysctl_iflist_ifm(ifp, &info, w, len);
			if (error)
				goto done;
		}
		while ((ifa = TAILQ_NEXT(ifa, ifa_link)) != NULL) {
			if (af && af != ifa->ifa_addr->sa_family)
				continue;
#if 0                        
			if (prison_if(w->w_req->td->td_ucred,
			    ifa->ifa_addr) != 0)
				continue;
#endif                        
			info.rti_info[RTAX_IFA] = ifa->ifa_addr;
			info.rti_info[RTAX_NETMASK] = ifa->ifa_netmask;
			info.rti_info[RTAX_BRD] = ifa->ifa_dstaddr;
			len = rt_msg2(RTM_NEWADDR, &info, NULL, w);
			if (w->w_req && w->w_tmem) {
#if 0                
				if (w->w_op == NET_RT_IFLISTL)
					error = sysctl_iflist_ifaml(ifa, &info,
					    w, len);
				else
#endif                    
					error = sysctl_iflist_ifam(ifa, &info,
					    w, len);
				if (error)
					goto done;
			}
		}
		IF_ADDR_RUNLOCK(ifp);
		info.rti_info[RTAX_IFA] = info.rti_info[RTAX_NETMASK] =
			info.rti_info[RTAX_BRD] = NULL;
	}
done:
	if (ifp != NULL)
		IF_ADDR_RUNLOCK(ifp);
	IFNET_RUNLOCK();
	return (error);
}

#if 0
static int
sysctl_ifmalist(int af, struct walkarg *w)
{
	struct ifnet *ifp;
	struct ifmultiaddr *ifma;
	struct	rt_addrinfo info;
	int	len, error = 0;
	struct bsd_ifaddr *ifa;

	bzero((caddr_t)&info, sizeof(info));
	IFNET_RLOCK();
	TAILQ_FOREACH(ifp, &V_ifnet, if_link) {
		if (w->w_arg && w->w_arg != ifp->if_index)
			continue;
		ifa = ifp->if_addr;
		info.rti_info[RTAX_IFP] = ifa ? ifa->ifa_addr : NULL;
		IF_ADDR_RLOCK(ifp);
		TAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {
			if (af && af != ifma->ifma_addr->sa_family)
				continue;
#if 0                        
			if (prison_if(w->w_req->td->td_ucred,
			    ifma->ifma_addr) != 0)
				continue;
#endif                        
			info.rti_info[RTAX_IFA] = ifma->ifma_addr;
			info.rti_info[RTAX_GATEWAY] =
			    (ifma->ifma_addr->sa_family != AF_LINK) ?
			    ifma->ifma_lladdr : NULL;
			len = rt_msg2(RTM_NEWMADDR, &info, NULL, w);
			if (w->w_req && w->w_tmem) {
				struct ifma_msghdr *ifmam;

				ifmam = (struct ifma_msghdr *)w->w_tmem;
				ifmam->ifmam_index = ifma->ifma_ifp->if_index;
				ifmam->ifmam_flags = 0;
				ifmam->ifmam_addrs = info.rti_addrs;
				error = SYSCTL_OUT(w->w_req, w->w_tmem, len);
				if (error) {
					IF_ADDR_RUNLOCK(ifp);
					goto done;
				}
			}
		}
		IF_ADDR_RUNLOCK(ifp);
	}
done:
	IFNET_RUNLOCK();
	return (error);
}
#endif
// struct sysctl_oid *oidp, void *arg1,	intptr_t arg2, struct sysctl_req *req
int
sysctl_rtsock(SYSCTL_HANDLER_ARGS)
{
	int	*name = (int *)arg1;
	u_int	namelen = arg2;
	struct radix_node_head *rnh = NULL; /* silence compiler. */
	int	i, lim, error = EINVAL;
	u_char	af;
	struct	walkarg w;

	name ++;
	namelen--;
	if (req->newptr)
		return (EPERM);
	if (namelen != 3)
		return ((namelen < 3) ? EISDIR : ENOTDIR);
	af = name[0];
	if (af > AF_MAX)
		return (EINVAL);
	bzero(&w, sizeof(w));
	w.w_op = name[1];
	w.w_arg = name[2];
	w.w_req = req;
#if 0
	error = sysctl_wire_old_buffer(req, 0);
	if (error)
		return (error);
#endif        
	switch (w.w_op) {

	case NET_RT_DUMP:
#if 0        
	case NET_RT_FLAGS:
#endif        
		if (af == 0) {			/* dump all tables */
			i = 1;
			lim = AF_MAX;
		} else				/* dump only one table */
			i = lim = af;
#if 0
		/*
		 * take care of llinfo entries, the caller must
		 * specify an AF
		 */
		if (w.w_op == NET_RT_FLAGS &&
		    (w.w_arg == 0 || w.w_arg & RTF_LLINFO)) {
			if (af != 0)
				error = lltable_sysctl_dumparp(af, w.w_req);
			else
				error = EINVAL;
			break;
		}
#endif        
		/*
		 * take care of routing entries
		 */
		for (error = 0; error == 0 && i <= lim; i++) {
#if 0                    
                    rnh = rt_tables_get_rnh(req->td->td_proc->p_fibnum, i);
#endif                    
                    // Note: We only support a single fib for the moment
			rnh = rt_tables_get_rnh(0, i);
			if (rnh != NULL) {
				RADIX_NODE_HEAD_RLOCK(rnh); 
			    	error = rnh->rnh_walktree(rnh,
				    sysctl_dumpentry, &w);
				RADIX_NODE_HEAD_RUNLOCK(rnh);
			} else if (af != 0)
				error = EAFNOSUPPORT;
		}
		break;

	case NET_RT_IFLIST:
#if 0        
	case NET_RT_IFLISTL:
#endif        
		error = sysctl_iflist(af, &w);
		break;

#if 0        
	case NET_RT_IFMALIST:
		error = sysctl_ifmalist(af, &w);
		break;
#endif        
	}
	if (w.w_tmem)
		free(w.w_tmem);
	return (error);
}
#if 0
SYSCTL_NODE(_net, PF_ROUTE, routetable, CTLFLAG_RD, sysctl_rtsock, "");
#endif
/*
 * Definitions of protocols supported in the ROUTE domain.
 */

extern struct domain routedomain;		/* or at least forward */

static struct protosw routesw[] = {
    initialize_with([] (protosw& x) {
	x.pr_type =		SOCK_RAW;
	x.pr_domain =		&routedomain;
	x.pr_flags =		PR_ATOMIC|PR_ADDR;
	x.pr_output =		route_output;
	x.pr_ctlinput =		raw_ctlinput;
	x.pr_init =		raw_init;
	x.pr_usrreqs =		&route_usrreqs;
    }),
};

struct domain routedomain = initialize_with([] (domain& x) {
	x.dom_family =		PF_ROUTE;
	x.dom_name =		 "route";
	x.dom_protosw =		routesw;
	x.dom_protoswNPROTOSW =	&routesw[sizeof(routesw)/sizeof(routesw[0])];
});

VNET_DOMAIN_SET(route);
