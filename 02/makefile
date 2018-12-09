#makefile

compiler=gcc

senderSRCS :=../common/common.c sender2.c
senderTARGET :=sender2

receiverSRCS :=../common/common.c receiver2.c
receiverTARGET := receiver2

all:
	$(compiler) -o $(senderTARGET) $(senderSRCS)
	$(compiler) -o $(receiverTARGET) $(receiverSRCS)
#	cleano

#all:$(TARGET) cleano

#cleano:
#	rm -rf *.o

.PHONY:clean
clean:
	rm -rf $(senderTARGET) 1551445* network* fifo*
	rm -rf $(receiverTARGET) 
