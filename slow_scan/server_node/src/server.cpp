#include <ros/ros.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define M 6
#define N 7

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
            if(token[1] == 'c'){
              CMD[0] = token[7]; CMD[1] = token[8]; CMD[2] = token[9]; 
              std::cout << CMD << std::endl;
            }else if(token[1] == 's'){
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
            }else if(token[1] == 'x'){
              std::string ll = token;
              int idx = 4;
              for(;;idx++)
                if(ll[idx] > '9' || ll[idx] < '0')
                  break;
              char x_char[idx - 4];
              for(int i = 0; i < sizeof(x_char); i++)
                X += (token[i+4] - '0') * pow(10, sizeof(x_char)-1-i);
              std::cout << X << std::endl;
              }else if(token[7] != 'Z'){
                if(token[10] != 'L'){
                  printf("wrong token");
                  std::cout << token << std::endl;
                }
              }
            token = strsep(&input, delim);
          }
          free(input);
        }

        switch (CMD[2]){
          case '0':
            ROS_INFO("got CMD 0");
            //CMD_SC0();
            break;
          case '1':
            ROS_INFO("got CMD 1");
            //CMD_SC0();
            break;
          default:
            break;
        }



        std::string send_repo = "{\"start\":\"LS\",";
        send_repo += send_stamp;
        send_repo += "\"cmd\":\"S";send_repo += CMD[1];send_repo += CMD[2];
        send_repo += "\",";
        send_repo += "\"DATA\":\"";
        for(int i=0; i<M; i++){
          for(int j=0; j<N; j++){
            send_repo += DATA[i][j];if(j!= N-1)send_repo += ",";else send_repo += "?";
          }
        }
        send_repo += "\",\"end\":\"ZG\"}";
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
        memset(&buffer, 0, sizeof(buffer));
        close(cli_sock);
        ros::param::set("s1_connect_sta", false);
        ros::Duration(0.1).sleep();
    }
    close(sock);
    return 0;
  }

