//
// Created by jiaxv on 23-9-28.
//

#include "ldacs_gsc.h"
// void run_gsc(sgw_conn_t *sgw_conn){
//     if(!sgw_conn){
//         log_error("Remote SGW server not open! Exit.....\n");
//         exit(0);
//     }
//     gsc_sgw_conn = sgw_conn;
//     init_heap_desc(&hd_conns);
//
//     ld_u_c_map = hashmap_new(sizeof(ld_unit_conn_node_t), 0, 0, 0,
//                              u_c_node_hash, NULL, NULL, NULL);
//     net_fd = server_entity_setup(LD_GSC);
//
//     //test_send_to_sgw(sgw_conn);
//
//     int nfds;
//     int i;
//     while (TRUE) {
//         nfds = core_epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, 20);
//
//         if (nfds == ERROR) {
//             // if not caused by signal, cannot recover
//             ERR_ON(errno != EINTR, "core_epoll_wait");
//         }
//
//         /* processing ready fd one by one */
//         for (i = 0; i < nfds; i++) {
//             struct epoll_event *curr_event = epoll_events + i;
//             int fd = *((int *)(curr_event->data.ptr));
//
//             if (fd == net_fd) {
//                 multi_ldacs_conn_accept();  /* gs-gsc accept */
//             }else {
//                 basic_conn_t **bcp = curr_event->data.ptr;
//                 int status;
//                 assert(bcp != NULL);
//
//                 if((*bcp)->rp->l_r == (LD_GSC)){
//                     if (connecion_is_expired(bcp))
//                         continue;
//                 }
//
//                 if (curr_event->events & EPOLLIN) {  //recv
//                     status = request_handle(bcp);
//                 }
//                 if (curr_event->events & EPOLLOUT) { //send
//                     status = response_handle(bcp);
//                 }
//
//                 if((*bcp)->rp->l_r == (LD_GSC)){
//                     if (status == ERROR)
//                         connecion_set_expired(bcp);
//                     else{
//                         connecion_set_reactivated(bcp);
//                     }
//                 }
//             }
//         }
//         server_connection_prune();
//     }
//     close(epoll_fd);
//     server_shutdown();
// }
