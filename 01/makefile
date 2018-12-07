#makefile

compiler=gcc

senderSRCS :=../common/common.c sender1.c
senderTARGET :=sender1

receiverSRCS :=../common/common.c receiver1.c
receiverTARGET := receiver1

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
