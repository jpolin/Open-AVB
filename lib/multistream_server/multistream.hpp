/*
 * Author: Joe Polin
 *
 * Description: Have a talker talk on multiple streams through same NIC.
 */

#ifndef MULTISTREAM_HPP_
#define MULTISTREAM_HPP_

// Standard libs
#include <queue>
#include <stdint.h>
#include <vector>
//#include <thread>
#include <stdlib.h>
#include <exception>

// Networking
#include <boost/asio.hpp>

using namespace boost::asio::ip;

/*
 * Class that listens on a specific UDP port, buffers messages, and returns
 * them to client.
 */
class loopback_buffer {

	// Hold pointers to messages
	std::queue<void *> msg_queue;

	// Where to listen
	unsigned int port;

	// Max size according to stream reservation
	size_t max_size;
	char *buffer;

	// UDP socket
	udp::socket sock;

public:
	// Initialize with port and max size (-1 for none)
	loopback_buffer(unsigned int _port, size_t _max_size, boost::asio::io_service &io_serv);

	~loopback_buffer(){
		delete buffer;
	}

	// Start listening on port
	void run();

	// Get next buffer on queue
	// IMPORTANT: Caller is expected to unallocate
	void *get_next_packet();

	// Manage queue
	size_t get_buffer_size() { return msg_queue.size(); }
	void flush_buffer();

	// Misc.
	unsigned int get_port(){ return port; }


private:

	void listen();
	void handle_msg(size_t nbytes);
};

// Store and transfer all info needed to define stream
enum avb_class {CLASS_A, CLASS_B};
struct stream_info {
	size_t max_packet_size; // bytes
	size_t packets_per_int; // packets per interval
	avb_class priority_class; // CLASS_A or CLASS_B
	uint8_t dest_mac[6]; // Destination mac address
	uint8_t send_max[6]; // Must associate with own interface
};

/*
 * Class that stores and manages loopback_buffers. It maintains knowledge of
 * the max bandwidth etc. associated with each loopback buffer (defined by
 * stream_info struct above)
 */

/*
class multistream_server {

	// Extend stream info to hold buffer pointer as well
	struct stream : stream_info {
		loopback_buffer *buffer;
	};
	std::vector<stream_info *> lb_buffers;

public:
	// Return true on success
	bool add_stream(struct stream_info sinfo, unsigned int loopback_port);

	// Calculate if bandwidth is over-used (note: only checks own egress port(s))
	bool verify_reservations();

	// Makes reservations (only if SRP is being used)
	bool make_reservations();

	// Start (launch threads and call run() on all loopback buffers). Also serve
	// them according to their priority class and reservation
	bool run();

	// Destructor; unallocate everything and close threads
	~multistream_server();

};
*/


#endif /* MULTISTREAM_HPP_ */
