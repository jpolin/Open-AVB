#include "multistream.hpp"

loopback_buffer::loopback_buffer(unsigned int _port, size_t _max_size, boost::asio::io_service &io_serv) :
	/* Save user-defined params */ port(_port), max_size(_max_size),
	/* Network */ sock(io_serv, udp::endpoint(udp::v4(), port))
{
	buffer = new char[max_size];
}

// Return pointer to next packet; client must free it when done
void *loopback_buffer::get_next_packet(){
	void *ret_val = NULL;
	if (!msg_queue.empty()) {
		ret_val = msg_queue.front();
		msg_queue.pop();
	}
	return ret_val;
}

// Start our server and start the io_service (in Boost)
void loopback_buffer::run(){
	listen();
//	io_serv.run();
}

void loopback_buffer::listen(){
	// Start listening on loopback port and adding to queue
	sock.async_receive(
		/*buffer: */ boost::asio::buffer(buffer, max_size),
		/*callback: */ [this](boost::system::error_code ec, size_t nbytes)
		{
			this->handle_msg(nbytes);
		}
	);
}

// Callback for getting messages
void loopback_buffer::handle_msg(size_t nbytes){
	printf("Received message of size %zu bytes on port %u \n", nbytes, port);

	// TODO: Any packaging of packet before sending later; might as well
	// do that in a thread (ie here)

	// Add packet to buffer
	char *new_buffer = new char[nbytes];
	memcpy(new_buffer, buffer, nbytes);
	msg_queue.push(new_buffer);

	// Listen for next packet
	run();
}

// Pop everything off
void loopback_buffer::flush_buffer(){
	while (!msg_queue.empty()) msg_queue.pop();
}



