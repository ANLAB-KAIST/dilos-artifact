/*-
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)in_proto.c	8.2 (Berkeley) 2/9/95
 */

#include <sys/cdefs.h>

#include <osv/initialize.hh>
#include <bsd/porting/netport.h>

#include <bsd/sys/sys/param.h>
#include <bsd/sys/sys/socket.h>
#include <bsd/sys/sys/domain.h>
#include <bsd/sys/sys/protosw.h>
#include <bsd/sys/sys/queue.h>

/*
 * While this file provides the domain and protocol switch tables for IPv4, it
 * also provides the sysctl node declarations for net.inet.* often shared with
 * IPv6 for common features or by upper layer protocols.  In case of no IPv4
 * support compile out everything but these sysctl nodes.
 */
#ifdef INET
#include <bsd/sys/net/if.h>
#include <bsd/sys/net/route.h>
#ifdef RADIX_MPATH
#include <bsd/sys/net/radix_mpath.h>
#endif
#include <bsd/sys/net/vnet.h>
#endif /* INET */

#if defined(INET) || defined(INET6)
#include <bsd/sys/netinet/in.h>
#endif

#ifdef INET
#include <bsd/sys/netinet/in_systm.h>
#include <bsd/sys/netinet/in_var.h>
#include <bsd/sys/netinet/ip.h>
#include <bsd/sys/netinet/ip_var.h>
#include <bsd/sys/netinet/ip_icmp.h>
#include <bsd/sys/netinet/igmp_var.h>
#include <bsd/sys/netinet/tcp.h>
#include <bsd/sys/netinet/tcp_timer.h>
#include <bsd/sys/netinet/tcp_var.h>

#if 0
#include <netinet/ip_encap.h>
#endif

#include <bsd/sys/netinet/udp.h>
#include <bsd/sys/netinet/udp_var.h>
/*
 * TCP/IP protocol family: IP, ICMP, UDP, TCP.
 */

static struct pr_usrreqs nousrreqs;

#ifdef IPSEC
#include <netipsec/ipsec.h>
#endif /* IPSEC */

#ifdef SCTP
#include <netinet/in_pcb.h>
#include <netinet/sctp_pcb.h>
#include <netinet/sctp.h>
#include <netinet/sctp_var.h>
#endif /* SCTP */

FEATURE(inet, "Internet Protocol version 4");

extern	struct domain inetdomain;

/* Spacer for loadable protocols. */
#define IPPROTOSPACER   			\
	initialize_with([](protosw& x) {	\
	x.pr_domain =		&inetdomain;	\
	x.pr_protocol =		PROTO_SPACER;	\
	x.pr_usrreqs =		&nousrreqs;	\
})

struct protosw inetsw[] = {
  initialize_with([] (protosw& x) {
    x.pr_type =      0;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_IP;
    x.pr_init =      ip_init;
#ifdef VIMAGE
    x.pr_destroy =       ip_destroy;
#endif
    x.pr_slowtimo =      ip_slowtimo;
    x.pr_drain =     ip_drain;
    x.pr_usrreqs =       &nousrreqs;
}),
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_DGRAM;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_UDP;
    x.pr_flags =     PR_ATOMIC|PR_ADDR;
    x.pr_input =     udp_input;
    x.pr_ctlinput =      udp_ctlinput;
    x.pr_ctloutput =     udp_ctloutput;
    x.pr_init =      udp_init;
#ifdef VIMAGE
    x.pr_destroy =       udp_destroy;
#endif
    x.pr_usrreqs =       &udp_usrreqs;
}),
  initialize_with([] (protosw& x) {
	x.pr_type =		SOCK_STREAM;
	x.pr_domain =		&inetdomain;
	x.pr_protocol =		IPPROTO_TCP;
	x.pr_flags =		PR_CONNREQUIRED|PR_IMPLOPCL|PR_WANTRCVD;
	x.pr_input =		tcp_input;
	x.pr_ctlinput =		tcp_ctlinput;
	x.pr_ctloutput =		tcp_ctloutput;
	x.pr_init =		tcp_init;
#ifdef VIMAGE
	x.pr_destroy =		tcp_destroy;
#endif
	x.pr_slowtimo =		tcp_slowtimo;
	x.pr_drain =		tcp_drain;
	x.pr_usrreqs =		&tcp_usrreqs;
}),
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_RAW;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_RAW;
    x.pr_flags =     PR_ATOMIC|PR_ADDR;
    x.pr_input =     rip_input;
    x.pr_ctlinput =      rip_ctlinput;
    x.pr_ctloutput =     rip_ctloutput;
    x.pr_usrreqs =       &rip_usrreqs;
}),
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_RAW;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_ICMP;
    x.pr_flags =     PR_ATOMIC|PR_ADDR|PR_LASTHDR;
    x.pr_input =     icmp_input;
    x.pr_ctloutput =     rip_ctloutput;
    x.pr_usrreqs =       &rip_usrreqs;
}),
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_RAW;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_IGMP;
    x.pr_flags =     PR_ATOMIC|PR_ADDR|PR_LASTHDR;
    x.pr_input =     igmp_input;
    x.pr_ctloutput =     rip_ctloutput;
    x.pr_fasttimo =      igmp_fasttimo;
    x.pr_slowtimo =      igmp_slowtimo;
    x.pr_usrreqs =       &rip_usrreqs;
}),
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_RAW;
    x.pr_domain =        &inetdomain;
    x.pr_protocol =      IPPROTO_RSVP;
    x.pr_flags =     PR_ATOMIC|PR_ADDR|PR_LASTHDR;
    x.pr_input =     rsvp_input;
    x.pr_ctloutput =     rip_ctloutput;
    x.pr_usrreqs =       &rip_usrreqs;
}),
/* Spacer n-times for loadable protocols. */
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
IPPROTOSPACER,
/* raw wildcard */
  initialize_with([] (protosw& x) {
    x.pr_type =      SOCK_RAW;
    x.pr_domain =        &inetdomain;
    x.pr_flags =     PR_ATOMIC|PR_ADDR;
    x.pr_input =     rip_input;
    x.pr_ctloutput =     rip_ctloutput;
    x.pr_init =      rip_init;
#ifdef VIMAGE
    x.pr_destroy =       rip_destroy;
#endif
    x.pr_usrreqs =       &rip_usrreqs;
})
};

extern int in_inithead(void **, int);
extern int in_detachhead(void **, int);

struct domain inetdomain = initialize_with([] (domain& x) {
	x.dom_family =		AF_INET;
	x.dom_name =		"internet";
	x.dom_protosw =		inetsw;
	x.dom_protoswNPROTOSW =	&inetsw[sizeof(inetsw)/sizeof(inetsw[0])];
#ifdef RADIX_MPATH
	x.dom_rtattach =		rn4_mpath_inithead;
#else
	x.dom_rtattach =		in_inithead;
#endif
#ifdef VIMAGE
	x.dom_rtdetach =		in_detachhead;
#endif
	x.dom_rtoffset =		32;
	x.dom_maxrtkey =		sizeof(struct bsd_sockaddr_in);
	x.dom_ifattach =		in_domifattach;
	x.dom_ifdetach =		in_domifdetach;
});

VNET_DOMAIN_SET(inet);
#endif /* INET */

SYSCTL_NODE(_net,      PF_INET,		inet,	CTLFLAG_RW, 0,
	"Internet Family");

SYSCTL_NODE(_net_inet, IPPROTO_IP,	ip,	CTLFLAG_RW, 0,	"IP");
SYSCTL_NODE(_net_inet, IPPROTO_ICMP,	icmp,	CTLFLAG_RW, 0,	"ICMP");
SYSCTL_NODE(_net_inet, IPPROTO_UDP,	udp,	CTLFLAG_RW, 0,	"UDP");
SYSCTL_NODE(_net_inet, IPPROTO_TCP,	tcp,	CTLFLAG_RW, 0,	"TCP");
#ifdef SCTP
SYSCTL_NODE(_net_inet, IPPROTO_SCTP,	sctp,	CTLFLAG_RW, 0,	"SCTP");
#endif
SYSCTL_NODE(_net_inet, IPPROTO_IGMP,	igmp,	CTLFLAG_RW, 0,	"IGMP");
#ifdef IPSEC
/* XXX no protocol # to use, pick something "reserved" */
SYSCTL_NODE(_net_inet, 253,		ipsec,	CTLFLAG_RW, 0,	"IPSEC");
SYSCTL_NODE(_net_inet, IPPROTO_AH,	ah,	CTLFLAG_RW, 0,	"AH");
SYSCTL_NODE(_net_inet, IPPROTO_ESP,	esp,	CTLFLAG_RW, 0,	"ESP");
SYSCTL_NODE(_net_inet, IPPROTO_IPCOMP,	ipcomp,	CTLFLAG_RW, 0,	"IPCOMP");
SYSCTL_NODE(_net_inet, IPPROTO_IPIP,	ipip,	CTLFLAG_RW, 0,	"IPIP");
#endif /* IPSEC */
SYSCTL_NODE(_net_inet, IPPROTO_RAW,	raw,	CTLFLAG_RW, 0,	"RAW");
