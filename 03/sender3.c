/* sender1.c */

#include "../common/common.h"
#include "../common/Timer.h"

int count = 1; //�����ļ�������

/**********************
 * �������ƣ�network_write
 * ��    �ܣ��������յ�enable_network_layer�źź�Ĵ�������
 			����1024�ֽ�д�빲���ļ����������㵽������·������ݴ���
 * ��    ������⵽���ź�
 * ��    �أ�
 * ˵    ����
 	(1)��xxx_network����ʹ��,xxx����һ��
 	(2)ʹ�ù����ļ�+�ź�ʵ�ֽ��̼����ݴ���
***********************/
void network_write(int sig)
{
	char sharedFilePath[PATHLENGTH]; //�����ļ���/·��network_datalink.share.xxxx
	int fd;							 //�����ļ�������
	char buf[MAX_PKT];				 //���ɵ�����
	if (count <= FILECOUNT)			 //��ʱֻ��5�ݣ�����۲�
	{
		getSharedFilePath(count, sharedFilePath);
		fd = open(sharedFilePath, O_CREAT | O_WRONLY, 0666);
		if (fd < 0) //�ļ�����/��ʧ��
		{
			printf("Shared file %s open fail!\n", sharedFilePath);
			return;
		}
		/*Ϊ�����ļ���д��*/
		if (lock_set(fd, F_WRLCK) == FALSE) //��д��ʧ��
			exit(-1);
		//continue;

		/*����1024�ֽ�����*/
		generateData(buf);

		/*�����ļ���д��Ҫ���ݵ�����*/
		if (write(fd, buf, MAX_PKT) < 0)
		{
			printf("Write share file %s fail!\n", sharedFilePath);
			lock_set(fd, F_UNLCK); //�˳�ǰ�Ƚ���
			exit(-1);
			//continue;
		}
		lock_set(fd, F_UNLCK); //�˳�ǰ�Ƚ���
		close(fd);
		enable_datalink_read(); //֪ͨ������·�������
		count++;
	}
}

/**********************
 * �������ƣ�datalink_read
 * ��    �ܣ�������·����յ�_DATALINK_RD�źź�Ĵ�������
			�Ӷ�Ӧ�Ĺ����ļ��ж�ȡ����㴫�ݵ�����
 * ��    ������⵽���ź�
 * ��    �أ�
 * ˵    ����
 	(1)��xxx_datalink����ʹ��,xxx����һ��
 	(2)ʹ�ù����ļ�+�ź�ʵ�ֽ��̼����ݴ���
***********************/
void datalink_read(int sig)
{
	Frame s;
	Packet buffer;
	char sharedFilePath[PATHLENGTH]; //�����ļ���/·��network_datalink.share.xxxx
	if (count <= FILECOUNT)
	{
		getSharedFilePath(count, sharedFilePath);

		/* ��������ȡ���ݣ�ͨ�������ļ�+�źŷ�ʽ */
		from_network_layer(&buffer, sharedFilePath);

		/* ����������ȡ�����ݰ���װ�롱����֡ */
		int i = 0;
		for (i = 0; i < MAX_PKT; i++)
			(s.info.data)[i] = (buffer.data)[i];

		/* ������֡���ݸ������ */
		to_physical_layer(&s);

		enable_network_write(); //֪ͨ�����д����
		count++;				//�ú���ֻ��������·���б����ã�
	}
}

//���������������
void network_layer()
{
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "sender1_network";
	prctl(PR_SET_NAME, new_name);

	/* 
			�ȴ�������·����깲���ļ����ݺ󷢡�д���źţ�
			�������1024�ֽ�����д�빲���ļ�
			ʵ������㵽������·������ݴ���
		*/

	mkdir("File", 0666);
	(void)signal(SIG_WR, network_write);

	while (1)
		sleep(1);
	exit(0);
}

//������·������������
void datalink_layer()
{
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "sender1_dtlink";
	prctl(PR_SET_NAME, new_name);

	/* 
		�ȴ������д�깲���ļ����ݺ󷢡������źţ�
		������������ݲ���װ������֡���͸������
	*/

	(void)signal(SIG_RD, datalink_read);

	while (1)
		sleep(1);

	exit(0);
}

//���������������
void physical_layer(int port)
{
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "sender1_physic";
	prctl(PR_SET_NAME, new_name);

	/* ʹ��TCP�׽���ͨ��ģ��������·ͨ�� */
	int server_sockfd;				//���������׽���
	int client_sockfd;				//�ͻ����׽���
	struct sockaddr_in my_addr;		//�������׽��ֵ�ַ�ṹ��
	struct sockaddr_in remote_addr; //�ͻ����׽��ֵ�ַ�ṹ��
	int sin_size;
	memset(&my_addr, 0, sizeof(my_addr)); //�������׽��ֵ�ַ�ṹ���ʼ��--����
	my_addr.sin_family = AF_INET;		  //������ͨ��Э��
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*�������������׽���*/
	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return 1;
	}

	/*����SO(_REUSEADDR*/
	int enable = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	/*���׽��ְ󶨵��������������ַ��*/
	if (bind(server_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("bind error");
		return 1;
	}

	/*�����������󣬼������г���Ϊ5*/
	if (listen(server_sockfd, 5) < 0)
	{
		perror("listen error");
		return 1;
	}

	sin_size = sizeof(struct sockaddr_in);
	/*�ȴ��ͻ����������󵽴�*/
	if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
	{
		perror("accept error");
		return 1;
	}

	enable_network_write(); //�����׼���ú�֪ͨӦ�ò�д����

	Frame s;
	Packet buffer;
	int count = 1;
	/*���տͻ��˵����ݲ����䷢�͸��ͻ��ˣ�resv�����յ����ֽ�����send���ط��͵��ֽ���*/
	while (1)
	{
		/* ��������·��ͨ�������ܵ���ȡ����֡ */
		physical_layer_from_datalink(&s);
		if (write(client_sockfd, &s, FRAMESIZE) < 0)
		{
			printf("Write socket fail!\n");
			continue;
		}
		count++;
		if (count > FILECOUNT)
			break;
	}
	while (1)
		sleep(1);
	close(client_sockfd);
	exit(0);
}

int main(int argc, char **argv)
{

	pid_t nt_pid; //�������̵�pid��
	pid_t dl_pid; //������·����̵�pid��
	pid_t ps_pid; //�������̵�pid��
	while ((nt_pid = fork()) < 0)
		sleep(1);

	if (nt_pid == 0)
	{
		//��������
		network_layer();
	}
	else if (nt_pid > 0) //������fork�ӽ���
	{

		while ((dl_pid = fork()) < 0)
			sleep(1);

		if (dl_pid == 0)
		{
			//������·�����
			datalink_layer();
		}
		else if (dl_pid > 0) //������fork�ӽ���
		{

			while ((ps_pid = fork()) < 0)
				sleep(1);

			if (ps_pid == 0)
			{
				//��������
				physical_layer(atoi(argv[1]));
			} //�����

			else if (ps_pid > 0) //������
				exit(0);
		}
	}
}
