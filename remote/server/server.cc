#include <linux/mman.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <map>
#include <remote/abort.hh>
#include <remote/common.hh>
#include <remote/defs.hh>

#define DEV_NAME "mlx5_0"
#define IB_PORT 1
#define GID_INDEX 1
#define CQ_SIZE 1
#define MAP_HUGE_SIZE MAP_HUGE_2MB

#include <time.h>

namespace remote {

static struct tm *log_tm;
static time_t log_time;
static char log_buff[20];

#define LOG(fmt, args...)                                              \
    log_time = time(0);                                                \
    log_tm = gmtime(&log_time);                                        \
    strftime(log_buff, sizeof(log_buff), "%Y-%m-%d %H:%M:%S", log_tm); \
    fprintf(stderr, "[%s] " fmt, log_buff, ##args)

struct client_ctx_t : peer_t {
    struct ibv_cq *cq;
    std::map<uint32_t, struct ibv_qp *> qps;
    struct ibv_mr *mr;
};

constexpr int EPOLL_SIZE = 64;

static std::map<uint32_t, client_ctx_t *> clients;
static ctx_t ctx;
static void *memory_region;
static size_t memory_region_len;

bool handle_fail(client_ctx_t &context) {
    int ret = 0;
    if (context.mr) {
        ret = ibv_dereg_mr(context.mr);
        context.mr = NULL;
        assert(ret == 0);
    }

    if (context.pd) {
        ret = ibv_dealloc_pd(context.pd);
        context.pd = NULL;
        assert(ret == 0);
    }

    if (context.cq) {
        ret = ibv_destroy_cq(context.cq);
        context.cq = NULL;
        assert(ret == 0);
    }
    return ret == 0;
}

bool handle_connect(request &req, response &resp) {
    client_ctx_t *client_ctx = new client_ctx_t();
    client_ctx->lid = req.connect.lid;
    memcpy(client_ctx->gid, &req.connect.gid, 16);

    client_ctx->cq = NULL;
    client_ctx->pd = NULL;
    client_ctx->mr = NULL;

    client_ctx->cq = ibv_create_cq(ctx.ib_ctx, CQ_SIZE, NULL, NULL, 0);
    if (client_ctx->cq == NULL) {
        abort("fail to create cq");
    }

    client_ctx->pd = ibv_alloc_pd(ctx.ib_ctx);
    if (client_ctx->pd == NULL) {
        abort("fail to alloc pd");
    }

    client_ctx->mr = ibv_reg_mr(client_ctx->pd, memory_region,
                                memory_region_len, cfg::server::mr_flags);

    if (!client_ctx->mr) {
        handle_fail(*client_ctx);
        resp.type = response_type::ERR;
        return false;
    }

    clients.insert(std::make_pair(client_ctx->mr->rkey, client_ctx));

    resp.type = response_type::OK;
    resp.rtype = request_type::CONNECT;
    resp.connect.lid = ctx.port_attr.lid;
    memcpy(resp.connect.gid, &ctx.gid, sizeof(ctx.gid));
    resp.connect.rkey = client_ctx->mr->rkey;
    resp.connect.start_addr = reinterpret_cast<uintptr_t>(client_ctx->mr->addr);
    resp.connect.len = client_ctx->mr->length;
    return true;
}

bool handle_new_qp(request &req, response &resp) {
    uint32_t rkey = req.new_qp.rkey;

    auto it = clients.find(rkey);

    if (it == clients.end()) {
        resp.type = response_type::ERR;
        return false;
    }
    client_ctx_t *&context = it->second;
    struct ibv_qp_init_attr qp_init_attr;
    memset(&qp_init_attr, 0, sizeof(qp_init_attr));

    // TODO: review parameters
    qp_init_attr.qp_type = IBV_QPT_RC;
    qp_init_attr.sq_sig_all = 0;
    qp_init_attr.send_cq = context->cq;
    qp_init_attr.recv_cq = context->cq;
    qp_init_attr.cap.max_send_wr = 1;
    qp_init_attr.cap.max_recv_wr = 1;
    qp_init_attr.cap.max_send_sge = 30;
    qp_init_attr.cap.max_recv_sge = 30;
    ibv_qp *qp = ibv_create_qp(context->pd, &qp_init_attr);
    if (!qp) {
        resp.type = response_type::ERR;
        return false;
    }

    context->qps.insert(std::make_pair(qp->qp_num, qp));

    resp.type = response_type::OK;
    resp.rtype = request_type::NEW_QP;
    resp.new_qp.qp_num = qp->qp_num;

    return true;
}
bool handle_connect_qp(request &req, response &resp) {
    uint32_t rkey = req.new_qp.rkey;
    auto it = clients.find(rkey);
    if (it == clients.end()) {
        resp.type = response_type::ERR;
        return false;
    }
    client_ctx_t *&context = it->second;

    auto it2 = context->qps.find(req.connect_qp.qp_num_remote);
    if (it2 == context->qps.end()) {
        resp.type = response_type::ERR;
        return false;
    }
    ibv_qp *&qp = it2->second;

    struct ibv_qp_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_INIT;
    attr.port_num = ctx.port_id;
    attr.pkey_index = cfg::pkey_index;
    attr.qp_access_flags = cfg::server::qp_access_flags;

    if (ibv_modify_qp(qp, &attr, cfg::server::qp_init_flags)) {
        resp.type = response_type::ERR;
        return false;
    }

    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = cfg::mtu;
    attr.dest_qp_num = req.connect_qp.qp_num_client;
    attr.rq_psn = cfg::rq_psn;
    attr.max_dest_rd_atomic = cfg::max_dest_rd_atomic;
    attr.min_rnr_timer = cfg::min_rnr_timer;
    attr.ah_attr.dlid = context->lid;
    attr.ah_attr.sl = cfg::sl;
    attr.ah_attr.src_path_bits = cfg::src_path_bits;
    attr.ah_attr.port_num = ctx.port_id;

    // Always have GID
    attr.ah_attr.is_global = 1;
    memcpy(&attr.ah_attr.grh.dgid, context->gid, 16);
    attr.ah_attr.grh.flow_label = cfg::flow_label;
    attr.ah_attr.grh.hop_limit = cfg::hop_limit;
    attr.ah_attr.grh.sgid_index = ctx.gid_idx;
    attr.ah_attr.grh.traffic_class = cfg::traffic_class;

    if (ibv_modify_qp(qp, &attr, cfg::server::qp_rtr_flags)) {
        printf("failed to modify QP, RTR\n");
        resp.type = response_type::ERR;
        return false;
    }

    memset(&attr, 0, sizeof(attr));
    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = cfg::timeout;
    attr.retry_cnt = cfg::retry_cnt;
    attr.rnr_retry = cfg::rnr_retry;
    attr.sq_psn = cfg::sq_psn;
    attr.max_rd_atomic = cfg::max_rd_atomic;

    if (ibv_modify_qp(qp, &attr, cfg::server::qp_rts_flags)) {
        printf("failed to modify QP, RTS\n");
        resp.type = response_type::ERR;
        return false;
    }

    resp.type = response_type::OK;
    resp.rtype = request_type::CONNECT_QP;

    return true;
}

int server_main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("memory_server <port_number> <memory_size in GB>\n");
        exit(EXIT_FAILURE);
    }
    int port_number = atoi(argv[1]);
    assert(port_number > 0);

    memory_region_len = (size_t)atoi(argv[2]) << 30;
    assert(memory_region_len > 0);

    init_ib(ctx, std::string(DEV_NAME), IB_PORT, GID_INDEX);

    LOG("Allocating memory...\n");
    memory_region =
        mmap(NULL, memory_region_len, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_SIZE, -1, 0);

    if (memory_region && memory_region == MAP_FAILED) {
        abort("mmap faild");
        return -1;
    }
    LOG("Allocating memory done: %p (%lx)\n", memory_region, memory_region_len);

    int listenfd = -1;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int sockopt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sockopt,
                   sizeof(sockopt)) == -1) {
        abort("Fail to set sock opt");
    }

    memset(&server_addr, 0x00, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_number);
    struct sockaddr *sock_addr = (struct sockaddr *)&server_addr;
    if (bind(listenfd, sock_addr, sizeof(server_addr)) < 0) {
        printf("Bind fail: %s\n", strerror(errno));
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 5) < 0) {
        printf("Listen fail");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    int epoll_fd = epoll_create(EPOLL_SIZE);
    struct epoll_event *events =
        (struct epoll_event *)calloc(EPOLL_SIZE, sizeof(struct epoll_event));
    struct epoll_event accept_event;
    accept_event.events = EPOLLIN;
    accept_event.data.fd = listenfd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listenfd, &accept_event);

    while (1) {
        int nready = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
        for (int i = 0; i < nready; i++) {
            if (events[i].events & EPOLLERR) {
                abort("epoll_wait returned EPOLLERR");
            }

            if (events[i].data.fd == listenfd) {
                int sockfd = -1;
                sockfd = accept(listenfd, NULL, 0);
                LOG("New client: %d\n", sockfd);
                if (sockfd < 0) {
                    close(listenfd);
                    abort("Server: accept failed.\n");
                }

                if (sockfd >= EPOLL_SIZE) {
                    abort("sockfd >= EPOLL_SIZE\n");
                }

                struct epoll_event event;
                memset(&event, 0, sizeof(event));

                event.data.fd = sockfd;
                event.events |= EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
                    abort("new client fail\n");
                }
            } else {
                int sockfd = events[i].data.fd;

                request req;
                memset(&req, 0, sizeof(req));

                int rc = 0;
                size_t total_read_bytes = 0, read_bytes = 0;
                while (!rc && total_read_bytes < sizeof(req)) {
                    read_bytes =
                        read(sockfd, &req, sizeof(req) - total_read_bytes);
                    if (read_bytes > 0)
                        total_read_bytes += read_bytes;
                    else if (read_bytes == 0)
                        break;
                    else
                        rc = read_bytes;
                    break;
                }

                // network to host
                req.convert();

                response resp;
                memset(&resp, 0, sizeof(resp));
                bool ret = false;
                switch (req.type) {
                    case request_type::CONNECT:
                        LOG("Handling CONNECT for %d\n", sockfd);
                        ret = handle_connect(req, resp);
                        LOG("Handling CONNECT for %d finished\n", sockfd);
                        break;
                    case request_type::NEW_QP:
                        LOG("Handling NEW_QP for %d\n", sockfd);
                        ret = handle_new_qp(req, resp);
                        LOG("Handling NEW_QP for %d finished\n", sockfd);
                        break;
                    case request_type::CONNECT_QP:
                        LOG("Handling CONNECT_QP for %d\n", sockfd);
                        ret = handle_connect_qp(req, resp);
                        LOG("Handling CONNECT_QP for %d finished\n", sockfd);
                        break;
                    default:
                        LOG("Unknown req %d for %d\n", (int)req.type, sockfd);
                        break;
                }
                assert(ret);

                resp.convert();

                int wc = 0;
                size_t total_write_bytes = 0, write_bytes = 0;
                while (!wc && total_write_bytes < sizeof(resp)) {
                    write_bytes =
                        write(sockfd, &resp, sizeof(resp) - total_write_bytes);
                    if (write_bytes > 0)
                        total_write_bytes += write_bytes;
                    else if (write_bytes == 0)
                        wc = 1;
                    else
                        wc = write_bytes;
                }
            }
        }
    }

    return 0;
}
}  // namespace remote

int main(int argc, char *argv[]) { return remote::server_main(argc, argv); }
