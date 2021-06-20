#include <ros/ros.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define M 100
#define N 140

int main(int argc, char** argv) {
    ros::init(argc, argv, "SN");
    ros::NodeHandle n;

    int sock = socket( AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srv_add;
    memset(&srv_add, 0, sizeof(srv_add));
    srv_add.sin_family = AF_INET;
    srv_add.sin_addr.s_addr = INADDR_ANY;
     srv_add.sin_port = htons(5300);
     bind(sock, (struct sockaddr*)&srv_add, sizeof(srv_add));
    listen(sock,10);
  
    struct sockaddr_in cli_add;
    socklen_t cli_add_size = sizeof(cli_add);
  
    
    bool CMD_ERR_FLAG;
    
    std::string DATA[M][N] = {};
    for(int i = 0; i< M; i++){
      for(int j= 0; j< N; j++){
        DATA[i][j] = "6";
      }
    }


    while(ros::ok()){
        ROS_INFO("waitting...");
        int cli_sock = accept(sock, (struct sockaddr*)&cli_add, &cli_add_size);
        ROS_INFO("catch!");
        ros::param::set("s1_connect_sta", true);

        char buffer[100] = {0};
        read(cli_sock, buffer, sizeof(buffer));
/*
  char get_buf[sizeof(buffer)] = "";
  memcpy((char*)&get_buf, buffer, sizeof(buffer));

char CMD[] = "SCX";
int Stamp = 0;int stamp_size = 0;std::string send_stamp;
int X = 0;
  if(!get_buf[sizeof(get_buf)-1]){
    char delim[] = ",";
    char *input = strdup(get_buf);
    char *token = strsep(&input, delim);
    while (token != NULL){
      if(token[2] == 'C'){
        CMD[0] = token[8]; CMD[1] = token[9]; CMD[2] = token[10]; 
        std::cout << CMD << std::endl;
      }else if(token[1] == 'S'){
        std::string ll = token;
        send_stamp = ll;
        int idx = 8;
        for(;;idx++)
          if(ll[idx] > '9' || ll[idx] < '0')
            break;
        stamp_size = idx - 8;
        char Stamp_char[stamp_size];
        for(int i = 0; i < sizeof(Stamp_char); i++)
          Stamp += (token[i+8] - '0') * pow(10, sizeof(Stamp_char)-1-i);
        std::cout << Stamp << std::endl;
      }else if(token[1] == 'X'){
        std::string ll = token;
        int idx = 4;
        for(;;idx++)
          if(ll[idx] > '9' || ll[idx] < '0')
            break;
        char x_char[idx - 4];
        for(int i = 0; i < sizeof(x_char); i++)
          X += (token[i+4] - '0') * pow(10, sizeof(x_char)-1-i);
        std::cout << X << std::endl;
      }else{
        printf("wrong token");
        std::cout << token << std::endl;
      }
      token = strsep(&input, delim);
    }
    free(input);


    bool local_show = false;
    ros::param::get("/local_show", local_show);
    if(local_show){
    sensor_msgs::PointCloud2 gcloud_msgs;
    pcl::toROSMsg(grayCloud, gcloud_msgs);
    gcloud_msgs.header.frame_id = "BBQ";
    pubber.publish(gcloud_msgs);
    pcl::PointCloud<pcl::PointXYZ>().swap(grayCloud);
    
    cv::Mat grayimg(FullM,FullN,0);
    for(int i=0; i < FullM; i++)
      for(int j=0; j < FullN; j++)
        grayimg.at<char>(i,j) = gray_Full[i][j];
    cv::namedWindow("gray",cv::WINDOW_NORMAL);
    cv::imshow("gray",grayimg);
    cv::waitKey(0);
    }
  }


  switch (CMD[2])
  {
  case '0':
    printf("got CMD 0");
    CMD_SC0();
    break;
  case '1':
    printf("got CMD 1");
    CMD_SC0();
    break;
  default:
    break;
  }


std::string STA[8] = {"1111","2222","33","444","5555","6666","777","8888"};
std::string CFG[2] = {"5","6"};


    std::string send_repo = "{\"CMD\":\"SCX\",";
    send_repo[10] = CMD[2];
    send_repo += send_stamp;
    send_repo += ",\"STA\":[";
for(int i=0; i<8; i++){
  send_repo += STA[i];
  if(i!= 7)send_repo += ",";
}
send_repo += "],\"CFG\":[";
send_repo += CFG[0];send_repo += ",";
send_repo += CFG[1];send_repo += "],";
send_repo += "\"DATA\":\"";

for(int i=0; i<M; i++){
  for(int j=0; j<N; j++){
    send_repo += DATA[i][j];if(j!= N-1)send_repo += ",";else send_repo += "?";
  }
}

send_repo += "\",\"RET\":\"0\"}";





    std::cout << std::endl << send_repo << "size is " << sizeof(send_repo) << std::endl;


int buf_idx = 0;
char sbuf[30000];
while(send_repo[buf_idx] != NULL){
sbuf[buf_idx] = send_repo[buf_idx];
buf_idx++;
}
sbuf[buf_idx] = 0;
    write(cli_sock, &sbuf, buf_idx+1);
    memset(&send_repo, 0, sizeof(send_repo));
    */
    memset(&buffer, 0, sizeof(buffer));
    close(cli_sock);
    ros::param::set("s1_connect_sta", false);
    ros::Duration(0.1).sleep();
  }
  close(sock);
  return 0;
}

