#makefile

compiler=gcc

senderSRCS :=../common/common.c sender3.c
senderTARGET :=sender3

receiverSRCS :=../common/common.c receiver3.c
receiverTARGET := receiver3

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
