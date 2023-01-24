#!/bin/bash 
cat lcm.txt|rev|awk '{n=(n==""?$1:((n*$1)/gcd(n,$1)));}END{print n;}function gcd(x,y){while(y!=0){t=y;y=x%y;x=t;}return x;}'
