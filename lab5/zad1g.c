#include <stdio.h>
char *napis = "Witajcie moi mili ...";
int main(){
  unsigned int index=0;
  for (;; ++index){
    printf("%c",napis[index]);
  }
  return 0;
}
