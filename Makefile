all:	OSHW_1 \
	OSHW_1_child
OSHW_1: OSHW_1.c
	gcc -g -Wall OSHW_1.c -o OSHW_1
OSHW_1_child: OSHW_1_child.c
	gcc -g -Wall OSHW_1_child.c -o OSHW_1_child