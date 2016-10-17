#include "multistream.hpp"

loopback_buffer::loopback_buffer(unsigned int _port, size_t _max_size, boost::asio::io_service &io_serv) :
	/* Save user-defined params */ port(_port), max_size(_max_size),
	/* Network */ sock(io_serv, udp::endpoint(udp::v4(), port))
{
	// Anything to do here?
}

// Copy next packet to client data and return size
size_t loopback_buffer::get_packet(void *buf, size_t buf_size){
	if (!msg_queue.empty()) {
		// Pop from queue
		struct buffered_packet packet = msg_queue.front();
		msg_queue.pop();

		// Check size (TODO: Put back, or cut off end?)
		if (packet.len > buf_size){
			printf("Buffered packet larger than client buffer. Ignoring for now.\n");
			delete packet.data;
			return 0;
		}
		// Copy data to client's buffer
		else {
			memcpy(buf, packet.data, packet.len);
			delete packet.data;
			return packet.len;
		}
	}
	// Queue empty
	else {
		return 0;
	}
}

void loopback_buffer::run(){
#ifdef DEBUG
	printf("Starting loopack buffer on port %u\n", port);
#endif
	listen();
}

void loopback_buffer::listen(){

#ifdef DEBUG
	printf("Assigning callback for port %u\n",port);
#endif

	// Start listening on loopback port and adding to queue
	uint8_t *buffer = new uint8_t[max_size];
	sock.async_receive(
		/*buffer: */ boost::asio::buffer(buffer, max_size),
		/*callback: */ [this, buffer](boost::system::error_code ec, size_t nbytes)
		{
			this->handle_msg(buffer, nbytes);
		}
	);
}

// Callback for getting messages
void loopback_buffer::handle_msg(uint8_t *buffer, size_t nbytes){

#ifdef DEBUG
	printf("Received message of size %zu bytes on port %u \n", nbytes, port);
#endif

	// TODO: Honor max_size

	// Convert to buffered packet
	struct buffered_packet new_packet;
	new_packet.len = nbytes;
	new_packet.data = new uint8_t[nbytes];
	memcpy(new_packet.data, buffer, nbytes);

	// Add to queue
	msg_queue.push(new_packet);

	// Listen for next packet
	listen();
}

// Pop everything off
void loopback_buffer::flush_buffer(){

#ifdef DEBUG
	printf("Flushing buffer\n");
#endif

	while (!msg_queue.empty()) {
		struct buffered_packet packet = msg_queue.front();
		msg_queue.pop();
		delete packet.data;
	}
}

bool multistream_server::add_stream(struct stream_info sinfo, unsigned int loopback_port){

	// Upgrade to stream struct
	struct stream new_stream(sinfo);

	// Map to a loopback_buffer
	new_stream.buffer = new loopback_buffer(loopback_port, MAX_PACKET_SIZE, io_serv);
	// TODO: Catch double port bind

	// Add to our list of loopbacks
	if (new_stream.priority_class == CLASS_A)
		lb_buffers_A.push_back(new_stream);
	else
		lb_buffers_B.push_back(new_stream);

	return true;
}

void multistream_server::serve_queues(std::vector<struct stream> &streams){
	printf("Serving queues\n");

	// Create counter for sending packets
	std::vector<size_t> count_master(streams.size());
	for (unsigned int i = 0; i < streams.size(); i++)
		count_master[i] = streams[i].packets_per_int;

	uint8_t buffer[MAX_PACKET_SIZE];

	// Set loop time
	const std::chrono::nanoseconds period_ns = 1e9;

	// Loop every xxx microseconds
	while (true) {
		const hrc::time_point start_time = hrc::now();

		// Cycle through all queues
		std::vector<size_t> count(count_master);
		bool all_sent = false;
		while (!all_sent){
			all_sent = true;
			for (unsigned int i = 0; i < streams.size(); i++){
				// All bandwidth used already
				if (!count[i]) continue;

				// Send and decrement counter
				size_t nbytes = streams[i].buffer->get_packet(buffer, MAX_PACKET_SIZE);
				if (!nbytes) continue;
				count[i]--;

				send_function((void*)&buffer, nbytes);

			}
		}
		const hrc::time_point sleep_time = period_ns - (hrc::now() - start);
#ifdef DEBUG
		printf("Sleeping for %u ns\n", sleep_time);
#endif
		boost::this_thread::sleep


	}
}

void multistream_server::run(){
	// Launch loopback ports
	for (struct stream &s : lb_buffers_A)
		s.buffer->run();
	for (struct stream &s : lb_buffers_B)
		s.buffer->run();


	// TODO also sort by interface?

	// Start serving the queues
	boost::thread serve_A(boost::bind(&multistream_server::serve_queues,this, lb_buffers_A));

	// Start the io_service
	io_serv.run();

	// Wait for them to end
	serve_A.join();
}

