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
#include <stdlib.h>
#include <exception>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

using namespace boost::asio::ip;

#define MAX_PACKET_SIZE 1024

/*
 * Store data with len known
 */
struct buffered_packet {
	size_t len = 0;
	uint8_t *data = NULL;
};

/*
 * Class that listens on a specific UDP port, buffers messages, and returns
 * them to client.
 */
class loopback_buffer {

	// Hold pointers to messages
	std::queue<struct buffered_packet> msg_queue;

	// Where to listen
	const unsigned int port;

	// Max size according to stream reservation
	const int max_size;

	// UDP socket
	udp::socket sock;

public:
	// Initialize with port and max size
	loopback_buffer(unsigned int _port, size_t _max_size,
			boost::asio::io_service &io_serv);

	~loopback_buffer(){
		printf("Deleting buffer\n");
		flush_buffer();
	}

	// Start listening on port
	void run();

	// Get next buffer on queue
	size_t get_packet(void *buf, size_t max_size);

	// Manage queue
	const size_t get_buffer_size() { return msg_queue.size(); }
	void flush_buffer();

	// Misc.
	const unsigned int get_port(){ return port; }


private:

	void listen();
	void handle_msg(uint8_t *buffer, size_t nbytes);
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
class multistream_server {

	// Extend stream info to hold buffer pointer as well
	struct stream : stream_info {
		loopback_buffer *buffer;
		stream(struct stream_info sinfo) : stream_info(sinfo) {}
	};
	// Separate vecs for each class (A and B)
	std::vector<struct stream> lb_buffers_A;
	std::vector<struct stream> lb_buffers_B;

	// Connects to listeners on loopback ports
	boost::asio::io_service io_serv;

public:
	// Return true on success
	bool add_stream(struct stream_info sinfo, unsigned int loopback_port);

	// Calculate if bandwidth is over-used (note: only checks own egress port(s))
	void verify_reservations() { printf("Not implemented\n"); }

	// Makes reservations (only if SRP is being used)
	void make_reservations() { printf("Not implemented\n"); }

	// Start (launch threads and call run() on all loopback buffers). Also serve
	// them according to their priority class and reservation
	void run();

	// Destructor; unallocate everything and close threads
//	~multistream_server();

private:

	// For a single interval, serve queues for class A/B
	void serve_queues(std::vector<struct stream> &streams);

};



#endif /* MULTISTREAM_HPP_ */
