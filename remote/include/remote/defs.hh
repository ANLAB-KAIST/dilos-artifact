#pragma once

#include <arpa/inet.h>
#include <byteswap.h>

#include <cstdint>

// ms: memory server
namespace remote {
using raddr_t = uintptr_t;

enum class request_type : uint8_t {
    CONNECT = 1,
    NEW_QP = 2,
    CONNECT_QP = 3,
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint16_t convert_field(uint16_t from) { return bswap_16(from); }
static inline uint32_t convert_field(uint32_t from) { return bswap_32(from); }
static inline uint64_t convert_field(uint64_t from) { return bswap_64(from); }
#endif

struct request {
    request_type type;
    union {
        struct {
            uint16_t lid;
            uint8_t gid[16];
        } connect;
        struct {
            uint32_t rkey;
        } new_qp;
        struct {
            uint32_t rkey;
            uint32_t qp_num_client;
            uint32_t qp_num_remote;
        } connect_qp;
    };

    inline void convert() {
#if __BYTE_ORDER == __LITTLE_ENDIAN

        switch (type) {
            case request_type::CONNECT:
                connect.lid = convert_field(connect.lid);
                break;
            case request_type::NEW_QP:
                new_qp.rkey = convert_field(new_qp.rkey);
                break;
            case request_type::CONNECT_QP:
                connect_qp.rkey = convert_field(connect_qp.rkey);
                connect_qp.qp_num_client =
                    convert_field(connect_qp.qp_num_client);
                connect_qp.qp_num_remote =
                    convert_field(connect_qp.qp_num_remote);
                break;
            default:
                break;
        }
#endif
    }
};

enum class response_type : uint8_t {
    OK = 0,
    ERR = 1,
};

struct response {
    response_type type;
    request_type rtype;
    union {
        struct {
            uint16_t lid;
            uint8_t gid[16];
            uint32_t rkey;
            uint64_t start_addr;
            uint64_t len;
        } connect;
        struct {
            uint32_t qp_num;
        } new_qp;
        struct {
        } connect_qp;
    };

    inline void convert() {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        if (type == response_type::ERR) return;

        switch (rtype) {
            case request_type::CONNECT:
                connect.lid = convert_field(connect.lid);
                connect.rkey = convert_field(connect.rkey);
                connect.start_addr = convert_field(connect.start_addr);
                connect.len = convert_field(connect.len);
                break;
            case request_type::NEW_QP:
                new_qp.qp_num = convert_field(new_qp.qp_num);
                break;
            default:
                break;
        }
#endif
    }
};
}  // namespace remote
