/*
 * main.cpp -- for testing only
 *
 *  Created on: Oct 10, 2016
 *      Author: jpolin
 */
#include "multistream.hpp"

int main(int argc, char** argv){
	boost::asio::io_service io_serv;
	loopback_buffer buf1(9090, 500, io_serv);
	buf1.run();
	loopback_buffer buf2(9091, 500, io_serv);
	buf2.run();
	io_serv.run();

}



