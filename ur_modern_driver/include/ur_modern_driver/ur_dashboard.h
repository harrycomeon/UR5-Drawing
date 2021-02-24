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

#ifndef UR_DASHBOARD_H_
#define UR_DASHBOARD_H_

#include "do_output.h"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <fcntl.h>
#include <sys/types.h>

// constant chars
static const char* EXPERT_USER_REQ = "setUserRole none\n";
static const char* EXPERT_USER_RES = "Setting user role: NONE\n";

static const char* RESTRICT_USER_REQ = "setUserRole restricted\n";
static const char* RESTRICT_USER_RES = "Setting user role: RESTRICTED\n";

static const char* UNLOCK_PROTECTIVE_REQ = "unlock protective stop\n";
static const char* UNLOCK_PROTECTIVE_RES = "Protective stop releasing\n";

static const char* CLOSE_SAFETY_POPUP_REQ = "close safety popup\n";
static const char* CLOSE_SAFETY_POPUP_RES = "closing safety popup\n";

static const char* CLOSE_POPUP_REQ = "close popup\n";
static const char* CLOSE_POPUP_RES = "closing popup\n";

static const char* POWER_UP_REQ = "power on\n";
static const char* POWER_UP_RES = "Powering on\n";

static const char* BRAKE_RELEASE_REQ = "brake release\n";
static const char* BRAKE_RELEASE_RES = "Brake releasing\n";

static const char* ROBOT_MODE_REQ = "robotmode\n";
static const char* RUNNING_MODE_RES = "Robotmode: RUNNING\n";

static const char* POPUP_REQ = "popup This robot is being controlled with ROS\n";
static const char* POPUP_RES = "showing popup\n";

class UrDashboard {
private:
	int dash_sockfd_;
	struct sockaddr_in dash_serv_addr_;
	struct hostent *server_;
	bool keepalive_;
	std::thread comThread_;
	int flag_;
	void run();

	bool dashCall(const char* &__req, const char* &__expected_res);

	std::mutex comm_locker__;

public:
	bool connected_;
	bool is_robot_ready__;

	UrDashboard(std::string host);
	void userRestricted();
	void userExpert();
	bool powerUp();
	bool releaseProtectiveStop();
	bool start();
	void halt();

};

#endif /* UR_DASHBOARD_H_ */