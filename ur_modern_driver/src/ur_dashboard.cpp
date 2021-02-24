/*
 * ur_dashboard.h, based on ur_communication.h, but without robot state..
 *
 * Copyright 2018 Carlos J. Rosales Gallegos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include "ur_modern_driver/ur_dashboard.h"

UrDashboard::UrDashboard(std::string host)
{
	bzero((char *) &dash_serv_addr_, sizeof(dash_serv_addr_));
	dash_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (dash_sockfd_ < 0) {
		print_fatal("ERROR opening socket pri_sockfd");
	}
	server_ = gethostbyname(host.c_str());
	if (server_ == NULL) {
		print_fatal("ERROR, unknown host");
	}
	dash_serv_addr_.sin_family = AF_INET;
	bcopy((char *) server_->h_addr, (char *)&dash_serv_addr_.sin_addr.s_addr, server_->h_length);
	dash_serv_addr_.sin_port = htons(29999);
	flag_ = 1;
	setsockopt(dash_sockfd_, IPPROTO_TCP, TCP_NODELAY, (char *) &flag_,
			sizeof(int));
	setsockopt(dash_sockfd_, IPPROTO_TCP, TCP_QUICKACK, (char *) &flag_,
			sizeof(int));
	setsockopt(dash_sockfd_, SOL_SOCKET, SO_REUSEADDR, (char *) &flag_,
			sizeof(int));
	fcntl(dash_sockfd_, F_SETFL, O_NONBLOCK);
	connected_ = false;
	keepalive_ = false;
}

bool UrDashboard::start() {
	keepalive_ = true;
	uint8_t buf[512];
	unsigned int bytes_read;
	std::string cmd;
	bzero(buf, 512);
	print_info("Connecting to dashboard...");
	fd_set writefds;
	struct timeval timeout;

	connect(dash_sockfd_, (struct sockaddr *) &dash_serv_addr_,
			sizeof(dash_serv_addr_));

	FD_ZERO(&writefds);
	FD_SET(dash_sockfd_, &writefds);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	select(dash_sockfd_ + 1, NULL, &writefds, NULL, &timeout);
	unsigned int flag_len;
	getsockopt(dash_sockfd_, SOL_SOCKET, SO_ERROR, &flag_, &flag_len);
	if (flag_ < 0)
	{
		print_fatal("Error connecting to dashboard interface");
		return false;
	}
	print_info("Dashboad interface: Got connection");
	connected_ = true;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	FD_ZERO(&writefds);
	FD_SET(dash_sockfd_, &writefds);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	select(dash_sockfd_ + 1, NULL, &writefds, NULL, &timeout);
	getsockopt(dash_sockfd_, SOL_SOCKET, SO_ERROR, &flag_, &flag_len);
	if (flag_ < 0) {
		print_fatal("Error connecting to dashboard interface");
		return false;
	}

	char read_buf[ ] = "Connected: Universal Robots Dashboard Server\n";
	size_t res = read(dash_sockfd_, read_buf, strlen(read_buf));

	if( !releaseProtectiveStop() )
	{
		print_fatal("Error powering up via dashboard interface");
		return false;
	}

	if( !dashCall(POPUP_REQ, POPUP_RES) )
	{
		print_fatal("Error poping up via dashboard interface");
		return false;
	}

	dashCall(RESTRICT_USER_REQ, RESTRICT_USER_RES);
	print_info("Dashboard in restricted mode");

	comThread_ = std::thread(&UrDashboard::run, this);

	return true;
}

void UrDashboard::halt() {
	keepalive_ = false;
	comThread_.join();
}

bool UrDashboard::dashCall(const char* &__req, const char* &__expected_res)
{
	if(!connected_)
		return false;

	comm_locker__.lock();

	if( write(dash_sockfd_, __req, strlen(__req)) != strlen(__req) )
		return false;

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	char read_buf[strlen(__expected_res)];
	size_t res = read(dash_sockfd_, read_buf, strlen(__expected_res));
	comm_locker__.unlock();

	std::string reply = read_buf;
	print_info("Dashboard: ");
	print_info(read_buf);

	std::string expected = __expected_res;
	return true; // (reply.find(expected) != std::string::npos);
}

void UrDashboard::userRestricted()
{
	dashCall(RESTRICT_USER_REQ, RESTRICT_USER_RES);
}

void UrDashboard::userExpert()
{
	dashCall(EXPERT_USER_REQ, EXPERT_USER_RES);
	dashCall(CLOSE_POPUP_REQ, CLOSE_POPUP_RES);
}

bool UrDashboard::releaseProtectiveStop()
{
	bool res = dashCall(UNLOCK_PROTECTIVE_REQ, UNLOCK_PROTECTIVE_RES);
	ros::Duration(1.0).sleep();
	res = res && dashCall(CLOSE_SAFETY_POPUP_REQ, CLOSE_SAFETY_POPUP_RES);
	ros::Duration(1.0).sleep();
	res = res && dashCall(CLOSE_POPUP_REQ, CLOSE_POPUP_RES);
	ros::Duration(1.0).sleep();
	res = res && powerUp();
	dashCall(POPUP_REQ, POPUP_RES);
	dashCall(RESTRICT_USER_REQ, RESTRICT_USER_RES);
	return res;
}

bool UrDashboard::powerUp()
{
	bool res = dashCall(POWER_UP_REQ, POWER_UP_RES);
	ros::Duration(3.0).sleep();
	res = res && dashCall(BRAKE_RELEASE_REQ, BRAKE_RELEASE_RES);
	ros::Duration(5.0).sleep();
	return res;
}

void UrDashboard::run() {
	uint8_t buf[2048];
	int bytes_read;
	bzero(buf, 2048);
	struct timeval timeout;

	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(dash_sockfd_, &writefds);

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(dash_sockfd_, &readfds);
	while (keepalive_)
	{
		while (connected_ && keepalive_)
		{
			if( comm_locker__.try_lock() )
			{
				select(dash_sockfd_ + 1, NULL, &writefds, NULL, NULL);
				unsigned int flag_len;
				getsockopt(dash_sockfd_, SOL_SOCKET, SO_ERROR, &flag_, &flag_len);
				if (flag_ < 0)
				{
					print_error("Error writing to port 29999. Is controller started? Will try to reconnect in 10 seconds...");
					connected_ = false;
					close(dash_sockfd_);
					continue;
				}
				else
				{
					connected_ = true;
				}

				comm_locker__.unlock();
			}

			//wait for some traffic so the UR socket doesn't die in version 3.1.
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		if (keepalive_)
		{
			//reconnect
			print_warning("Dashboard port: No connection. Is controller crashed? Will try to reconnect in 10 seconds...");
			dash_sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
			if (dash_sockfd_ < 0)
			{
				print_fatal("ERROR opening dashboard socket");
			}
			flag_ = 1;
			setsockopt(dash_sockfd_, IPPROTO_TCP, TCP_NODELAY, (char *) &flag_,
					sizeof(int));
			setsockopt(dash_sockfd_, IPPROTO_TCP, TCP_QUICKACK, (char *) &flag_,
					sizeof(int));
			setsockopt(dash_sockfd_, SOL_SOCKET, SO_REUSEADDR, (char *) &flag_,
					sizeof(int));
			fcntl(dash_sockfd_, F_SETFL, O_NONBLOCK);
			while (keepalive_ && !connected_)
			{
				std::this_thread::sleep_for(std::chrono::seconds(10));

				connect(dash_sockfd_, (struct sockaddr *) &dash_serv_addr_, sizeof(dash_serv_addr_));

				select(dash_sockfd_ + 1, NULL, &writefds, NULL, NULL);
				unsigned int flag_len;
				getsockopt(dash_sockfd_, SOL_SOCKET, SO_ERROR, &flag_, &flag_len);
				if (flag_ < 0)
				{
					print_error("Error re-connecting to port 29999. Is controller started? Will try to reconnect in 10 seconds...");
				}
				else
				{
					connected_ = true;
				}
			}
		}
	}

	//wait for some traffic so the UR socket doesn't die in version 3.1.
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	close(dash_sockfd_);
}
