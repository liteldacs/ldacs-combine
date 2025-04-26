//
// Created by 邹嘉旭 on 2025/4/24.
//

#include "ldacs_def.h"
#include "net/p2p.h"

l_err init_p2p_service(uint16_t server_port, peer_gs_t **peers, size_t peer_count) {
    for (int i = 0; i < peer_count; i++) {
        peer_gs_t *peer = peers[i];
        log_warn("Peer[%d] %s:%d UA: %d, listening on port: %d", i, peer->peer_addr, peer->peer_port, peer->peer_UA,
                 server_port);
        server_entity_setup(server_port, peer);
    }
    return LD_OK;
}
