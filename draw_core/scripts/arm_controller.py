#!/usr/bin/env python
import sys
import rospy
import time
import copy
import math

# moveit stuff
import moveit_commander

# msg stuff
import moveit_msgs.msg
import geometry_msgs.msg
from std_msgs.msg import String

class Arm_Contrl():
    def __init__(self):
        self.manipulator = moveit_commander.RobotCommander()
        # self.group_name = "manipulator_i5"
        self.group_name = "manipulator"
        self.group = moveit_commander.MoveGroupCommander(self.group_name)

        # initial settings
        reference = String()
        reference = "world"
        # reference = "base_link"
        self.group.set_pose_reference_frame(reference)
        self.group.allow_replanning(True)
        # self.group.allow_replanning(False)
        # self.group.set_max_velocity_scaling_factor(0.01)    
        self.group.set_max_velocity_scaling_factor(0.1)
        self.group.set_max_acceleration_scaling_factor(0.1)             
        # self.group.set_max_acceleration_scaling_factor(0.01)
        self.group.set_goal_orientation_tolerance(0.001)
        self.group.set_goal_position_tolerance(0.001)
        # self.group.set_goal_orientation_tolerance(0.01)
        # self.group.set_goal_position_tolerance(0.01)

        # self.group.set_goal_orientation_tolerance(10)
        # self.group.set_goal_position_tolerance(10)
        self.group.set_planning_time(6.0)
        # self.group.set_planning_time(10.0)

        # for drawing settings
        self.line_gap = 0.01

        # for real world settings
        self.up_pen_height = 0.45
        #self.low_pen_height = 0.35
        self.low_pen_height = 0.35

    def get_current_pos(self):
        return self.group.get_current_pose().pose

    def go_home(self):
        # goal = geometry_msgs.msg.Pose()
        # goal = self.group.get_current_pose().pose
        #joint_positions = [0.09833356546924307, -0.13036996652071509, -1.665223463667989, 0.010214026325672795, -1.604895446066772, 0.0984022748028983]  
	    #stimulation
        # joint_positions = [0.24122319430196537, -1.709900035599464, 1.6487993846047813, -1.5562544318195775, -1.5902356912751738, 1.8026332309843127]
	    #real
        joint_positions = [-0.40416080156435186, -1.9702099005328577, -2.441277090703146, -0.30787593523134404, 1.5638155937194824, 4.297321319580078]
        #self.group.set_joint_value_target(joint_positions)
        self.group.set_joint_value_target(joint_positions)
        # goal.position.x = -0.2
        # goal.position.y = -0.2
        # goal.position.z = 1.05 
        # goal.orientation.x = 0.0
        # goal.orientation.y = 0.0
        # goal.orientation.z = 0.0
        # goal.orientation.w = 1.0

        # self.group.set_pose_target(goal)
        plan = self.group.go(wait = True)
        time.sleep(1.0)
        self.group.clear_pose_targets()
        print("I am home now")


    def move(self, goal_state):
        # get current state
        current_state = geometry_msgs.msg.Pose()
        current_state = self.group.get_current_pose().pose
        self.group.set_pose_target(goal_state)

        # print("111111111111111")

        # plan and execute
        plan = self.group.go(wait = True)
        time.sleep(5.0)
        self.group.stop()
        self.group.clear_pose_targets()
        print("achieve move goal")

    def draw_line(self, end_point):
        # get current state
        current_state = geometry_msgs.msg.Pose()
        current_state = self.group.get_current_pose().pose

        line_points = []
        line_points.append(copy.deepcopy(current_state))
        # delta_x = end_point.position.x - current_state.position.x
        # delta_y = end_point.position.y - current_state.position.y
        # dist = math.sqrt(delta_x * delta_x + delta_y * delta_y)
        # # cos_theta = (dist * dist + delta_x * delta_x - delta_y * delta_y) / 2 * dist * delta_x
        # # sin_theta = math.sqrt(1 - cos_theta * cos_theta)
        # num_points = int(dist / self.line_gap)
        # print("dist", dist)
        # print("num_points", num_points)

        # temp_point = geometry_msgs.msg.Pose()
        # temp_point.position.z = current_state.position.x
        # temp_point.position.z = current_state.position.z +0.001
        # temp_point.position.z = current_state.position.y
        # temp_point.position.z = current_state.orientation.w
        # temp_point.position.z = current_state.orientation.x
        # temp_point.position.z = current_state.orientation.y
        # temp_point.position.z = current_state.orientation.z
        # temp_point = copy.deepcopy(current_state) 


        # for i in range(num_points):
        #     temp_point.position.x += delta_x / num_points
        #     temp_point.position.y += delta_y / num_points
        # line_points.append(temp_point)
        line_points.append(end_point)
        # print("111111111111111")       
        # draw the line
        (line_traj, fraction) = self.group.compute_cartesian_path(line_points, self.line_gap, 0,avoid_collisions= False)
        # print("line_traj pose", line_traj.joint_trajectory.points)
        self.group.execute(line_traj, wait=True)
        # print("2222222222222222")
        time.sleep(0.25)
        # time.sleep(q)
                
    def up_pen(self):   #Up the pen
        current_state_ = geometry_msgs.msg.Pose()
        current_state_ = self.group.get_current_pose().pose
        current_state_.position.z = self.up_pen_height
        self.draw_line(current_state_)
        # self.move(current_state_)        

    def down_pen(self):   #Down the pen
        current_state_ = geometry_msgs.msg.Pose()
        current_state_ = self.group.get_current_pose().pose
        current_state_.position.z = self.down_pen_height
        self.draw_line(current_state_)
        # self.move(current_state_)

# get current pose
if __name__=="__main__":
    moveit_commander.roscpp_initialize(sys.argv)     
    rospy.init_node('drawing_control', anonymous = True)
    
    
    # test plan  
    manipulator = moveit_commander.RobotCommander()
    # group_name = "manipulator_i5"
    group_name = "manipulator"
    group = moveit_commander.MoveGroupCommander(group_name)
    rate = rospy.Rate(2) # 0.5sec
    while not rospy.is_shutdown():
        p = group.get_current_pose().pose
        j = group.get_current_joint_values()
        print("current pose", p)
        print("current_joint_values", j)
        rate.sleep()




