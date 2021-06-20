#include "RMDS.h"
#include <iostream>
#include <algorithm>
#include "std_msgs/Int8.h"

#define GAP 3
#define Vel 70
#define init_speed 30
#define MAX_DEG 200



int dev_num;
bool dev_flag = false;
std::string port;
int flag_485 = 0;
double STA_Ang, END_Ang;
bool if_corr;
  
int main(int argc, char** argv) {
  ros::init(argc, argv, "RMDS_9015");
  ros::NodeHandle n("~");
  ros::Publisher pubber = n.advertise<std_msgs::Int8>("sig", 1000);
  std_msgs::Int8 flag;
  std::string para_name = "/devn/enable";
  n.param<int>("dev_num", dev_num, -1);
  n.param<std::string>("port",port,"uPort");
  para_name[4] = (char)dev_num + '0';
  RMDS_Motor::Ptr motor = std::make_shared<RMDS_Motor>(port, 1);
  int wrongTimes = 0;

  
while(ros::ok()){
  ros::param::get(para_name, dev_flag);
  if(!dev_flag)continue;
  
  ros::param::get("/motor_staAng", STA_Ang);
  END_Ang = STA_Ang + 190;
  int aim = 0;int speed = 0;bool revers_flag = false;
  double pos = motor->getCurrentPose();
  std::cout << pos << std::endl;
  if(abs(pos-STA_Ang) < GAP || (360 - abs(pos-STA_Ang)) < GAP){
    aim = END_Ang;speed = Vel;
  }else if(abs(pos-END_Ang) < GAP){
    aim = STA_Ang;speed = -Vel;revers_flag = true;
    ros::param::get("/corr_mode", if_corr);
    if(!if_corr)ros::param::set(para_name, false);
  }else{//init
    wrongTimes++;
    ROS_WARN("the %d wrong place at %lf deg!", wrongTimes, pos);
    if(pos-STA_Ang < 0 || pos-STA_Ang > (MAX_DEG+5)){
      aim = STA_Ang;speed = init_speed;
    }else{
      aim = STA_Ang;speed = -init_speed;
    }
  }
  flag.data = 0;
  pubber.publish(flag);
  flag.data = 1;
  double angle_now = 0;
  double start = ros::Time::now().toSec();
  do{
    motor->rotateTo(aim, speed);
    //pubber.publish(flag);
    angle_now = motor->getCurrentPose();
    //loop_rate.sleep();
  }while(!(abs(angle_now-aim) < GAP));
  std::cout << "speed: " << 190 / (ros::Time::now().toSec() - start) << std::endl;
  
  motor->rotate(0);
  
  flag.data = 2;
  if(revers_flag)flag.data = 3;
  pubber.publish(flag);
   ros::Duration(1).sleep();
}
  return 0;
}









bool RMDS_Motor::rotate(double speed) {
  //ROS_WARN("%d is using", dev_num);
  //ros::param::get("flag_485", flag_485);
  //while(flag_485 != 0)ros::param::get("flag_485", flag_485);
  //flag_485 = 1;
  //ros::param::set("flag_485", flag_485);
  
  if(isWorking || speed > RMDS_CMD::MAX_SPEED){
    //flag_485 = 0;
    //ros::param::set("flag_485", flag_485);
    //ROS_WARN("%d is quit", dev_num);
    return false;
  }
  isWorking = true;
  updateSpeed(speed);
  int32_t _speed = static_cast<int32_t>(speed*100);
  m_buffer.resize(RMDS_CMD::FRAME_LEN + sizeof(_speed) + 1);
  *(reinterpret_cast<int32_t*>(&m_buffer[RMDS_CMD::IDX_DATA_START])) = _speed;

  //flag_485 = 0;
  //ros::param::set("flag_485", flag_485);
  //ROS_WARN("%d is quit", dev_num);
  return send(RMDS_CMD::W_SPEED) > 0;
}


bool RMDS_Motor::rotateTo(double degree, double speed) {
  if(isWorking || speed > RMDS_CMD::MAX_SPEED || speed == 0 || degree < 0)return false;
  isWorking = true;
  updateSpeed(speed);
  uint32_t _speed = static_cast<uint32_t>(std::abs(speed)*100);
  uint16_t _degree = static_cast<uint16_t>(degree*100);
  size_t len = RMDS_CMD::FRAME_LEN + sizeof(_degree) + sizeof(_speed) + 3;

  m_buffer.resize(len);
  m_buffer[RMDS_CMD::IDX_DATA_START] = speed > 0 ? 0 : 1;
  *(reinterpret_cast<uint16_t*>(&m_buffer[RMDS_CMD::IDX_DATA_START+1])) = _degree;
  m_buffer[RMDS_CMD::IDX_DATA_START + sizeof(_degree) + 1] = 0; // NULL
  *(reinterpret_cast<uint32_t*>(&m_buffer[RMDS_CMD::IDX_DATA_START+sizeof(_degree)+2])) = _speed;

  return send(RMDS_CMD::W_SLANG2) > 0;
}


double RMDS_Motor::getCurrentPose() {
  if(isWorking)return -1;
  isWorking = true;
  m_buffer.resize(RMDS_CMD::FRAME_LEN);
  send(RMDS_CMD::R_SINGALLOOPANG);
  auto tmp = reinterpret_cast<uint16_t*>(&m_buffer[RMDS_CMD::IDX_DATA_START]);
  double angle = static_cast<double>(*tmp / 100.0);

  return angle;
}









RMDS_Motor::RMDS_Motor(std::string dev, uint8_t id, uint32_t baudrate):m_ser(dev, baudrate,  serial::Timeout::simpleTimeout(20)), m_buffer() , m_id(dev_num) , m_speed(180), m_acc(0), isWorking(false){
    m_buffer.reserve(100);
    m_buffer.push_back(RMDS_CMD::FRAME_HEAD);
    m_buffer.push_back(0);
    m_buffer.push_back(m_id);
    ROS_INFO("dev:%d",dev_num);
    //m_buffer.push_back(dev_num);
    m_buffer.push_back(0);
    m_buffer.push_back(0);

    if(!m_ser.isOpen()) {
        try {
            m_ser.open();
        } catch(serial::IOException& e) {
            std::cerr << "Unable to open port " << std::endl;
            exit(-1);
        }
    }
}







size_t RMDS_Motor::send(uint8_t cmd) {
  //ROS_WARN("%d is using", dev_num);
  m_buffer[RMDS_CMD::IDX_FRAME_HEAD] = RMDS_CMD::FRAME_HEAD;
  m_buffer[RMDS_CMD::IDX_CMD] = cmd;
  m_buffer[RMDS_CMD::IDX_ID] = m_id;
  m_buffer[RMDS_CMD::IDX_HEAD_CHK] = 0;
  if(m_buffer.size() > RMDS_CMD::FRAME_LEN) {
    m_buffer[RMDS_CMD::IDX_DATA_LEN] = m_buffer.size() - RMDS_CMD::FRAME_LEN - 1;
    m_buffer.back() = 0;
    for (size_t i = 0; i < m_buffer[RMDS_CMD::IDX_DATA_LEN]; i++)
      m_buffer.back() += m_buffer[RMDS_CMD::IDX_DATA_START + i];
  }else 
    m_buffer[RMDS_CMD::IDX_DATA_LEN] = 0;
  for (size_t i = 0; i < RMDS_CMD::IDX_HEAD_CHK; i++)
        m_buffer[RMDS_CMD::IDX_HEAD_CHK] += m_buffer[i];
  //for(uint8_t num : m_buffer)std::cout << std::hex << +num << " ";std::cout << std::endl;

  if(!m_ser.isOpen()) {
    try {
      m_ser.open();
    }catch(serial::IOException& e) {
      std::cerr << "Unable to open port " << std::endl;
      exit(-1);
    }
  }
  auto ret = m_ser.write(m_buffer);
  m_buffer.clear();
//double start = ros::Time::now().toSec();
    m_ser.read(m_buffer, m_buffer.capacity());
//std::cout << ros::Time::now().toSec() - start << std::endl;
  //ROS_WARN("%d is quit", dev_num);
  isWorking = false;
  return ret;
}


