/*
 * main.cpp -- for testing only
 *
 *  Created on: Oct 10, 2016
 *      Author: Joe Polin
 */

#include "multistream.hpp"
#include <signal.h>


int main(int argc, char** argv){

	struct stream_info s1;
	s1.max_packet_size = 128;
	s1.packets_per_int = 1;
	s1.priority_class = CLASS_A;
	uint8_t dest_mac1[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	uint8_t src_mac1[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	memcpy(s1.dest_mac, dest_mac1, 6);
	memcpy(s1.send_mac, src_mac1, 6);

	struct stream_info s2;
	s2.max_packet_size = 128;
	s2.packets_per_int = 1;
	s2.priority_class = CLASS_A;
	uint8_t dest_mac2[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x77};
	uint8_t src_mac2[] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	memcpy(s2.dest_mac, dest_mac2, 6);
	memcpy(s2.send_mac, src_mac2, 6);

	multistream_server mt_server([](void* buf, size_t msg_size)
		{
			printf("%.*s\n", (unsigned int)msg_size, (char *)buf);
		}
	);

	mt_server.add_stream(s1, 9090);
	mt_server.add_stream(s2, 9091);
	mt_server.run();

}



