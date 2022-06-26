#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#include "md5.h"

#define DEVICE_NODE "/dev/vchar_dev"

static inline void ignore_ret() {}

int open_chardev() {
  int fd = open(DEVICE_NODE, O_RDWR);
  if(fd < 0) {
    printf("Can not open the device file\n");
    exit(1);
  }
  return fd;
}

char * normalize_characters(char *user_buf) {
  int fd = open_chardev();
  int ret = read(fd, user_buf, strlen(user_buf) + 1);
  user_buf[ret] = '\0';
  close(fd);
  return user_buf;
}

uint8_t hex_to_uint8_t(char a, char b) {
  uint8_t rs = 0;
  if (a>='0' && a <= '9') {
    rs = rs + (a-48)*16;
  } else {
    rs = rs + (a-87)*16;
  }

  if (b>='0' && b<= '9') {
    rs = rs + (b-48);
  } else
    rs = rs + (b-87);

  return rs;
}

void print_hash(uint8_t *p){
  for(unsigned int i = 0; i < 16; ++i){
    printf("%02x", p[i]);
  }
  printf("\n");
}

bool login(){
  printf("Enter username: ");
  char username[256];
  ignore_ret(scanf(" %[^\n]s", username));
  char password[256];
  printf("Enter password: ");
  ignore_ret(scanf(" %[^\n]s", password));

  uint8_t * encrypted_pw = md5String(password);
  FILE* fp;
  fp = fopen("admin.txt", "r");
  char buff[256];
  ignore_ret(fgets(buff, 256, fp));

  char * un = buff;

  uint8_t i;
  for (i = 0; i<strlen(un)-1; i++){
    if (un[i] != username[i])
      return false;
  }


  ignore_ret(fgets(buff, 256, fp));
  printf("%s\n", buff);
  for (i = 0; i<16; i++) {
    if (encrypted_pw[i] != hex_to_uint8_t(buff[i*2], buff[i*2+1])) {
      return false;
    }
  }

  return true;
}

struct Student {
  int id;
  char * name;
};

const char *toString(struct Student st)
{
  char *rs = (char *)malloc(100 * sizeof(char));
  sprintf(rs, "%d-%s\n", st.id, st.name);
  return rs;
}

struct Student students[100];
int count = 0;

void addStudent(){
  printf("Enter ID: ");
  int id;
  ignore_ret(scanf("%d", &id));
  students[count].id = id;

  printf("Enter Name: ");
  char name[100];
  fflush(stdin);
  getchar();
  ignore_ret(scanf(" %[^\n]s", name));
  students[count].name = (char *)malloc(100 * sizeof(char));;
  strcpy(students[count].name, name);
  students[count].name = normalize_characters(students[count].name);
  printf("%s\n", toString(students[count]));
  count++;
}

void writeToFile() {
  FILE* fp;
  fp = fopen("students.txt", "w+");

  int i;
  for (i = 0; i < count; i++)
  {
    fprintf(fp, "%s", toString(students[i]));
  }
  fclose(fp);
}

int main() {
  int option = 0;
  while(1){
    printf("1.Login\n");
    printf("2.Quit\n");
    printf("Enter your option: ");
    ignore_ret(scanf("%d", &option));
    switch (option) {
      case 1:
        {
          if(login()){
            while (1) {
              printf("Login success\n");
              printf("1.Add student\n");
              printf("2.Export to file\n");
              printf("3.Quit\n");
              printf("Enter your option: ");
              ignore_ret(scanf("%d", &option));
              switch(option) {
                case 1:
                  addStudent();
                  break;
                case 2:
                  {
                    writeToFile();
                    break;
                  }
                case 3:
                  printf("Goodbye\n");
                  return 0;
                default:
                  printf("Invalid option\n");
                  break;
              }
            }

          } else {
            printf("Login failed\n");
          }
          break;
        }
      case 2:
        printf("Goodbye!");
        return 0;
      default:
        printf("Invalid option!\n");
        break;
    }
  }
}
