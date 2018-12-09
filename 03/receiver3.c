
/* receiver.c */

#include "../common/common.h"
#include "../common/Timer.h"

int count = 1;

int client_sockfd;
void physical_write(int sig)
{
	Frame r;
	if (count > FILECOUNT) //��ʱֻ��5��
		return;

	//generateData(r.info.data);//�������֡������,��װ�Ǵӷ��ͷ��յ���,֮���ⲿ�ָ�Ϊsocketͨ�ż���

	if (read(client_sockfd, &r, FRAMESIZE) < 0)
	{
		printf("TCP socket read fail!\n");
		return;
	}
	physical_layer_to_datalink(&r); //�����ݴ��ݵ�������·��,ͨ�������ܵ���ʽ
	printf("[Physical layer send]:%d\n", count);
	count++;
}

//������������������
void network_layer()
{
	mkdir("File", 0666);
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "recv1_network";
	prctl(PR_SET_NAME, new_name);

	Packet buffer;
	int count = 1;
	char fileName[PATHLENGTH];
	while (1)
	{
		enable_physical_write(); //֪ͨ�����д����
		getSharedFilePath(count, fileName);
		network_layer_from_datalink(&buffer, fileName);
		printf("[Network layer recv]:%d\n", count);
		count++;
		if (count > FILECOUNT)
			break;
	}
	while (1)
		sleep(1);
	exit(0);
}

//������·������������
void datalink_layer()
{
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "recv1_dtlink";
	prctl(PR_SET_NAME, new_name);
	Frame r;
	Packet buffer;
	int count = 1;
	while (1)
	{


		/* ��������������֡ */
		from_physical_layer(&r);
		printf("[Datalink layer recv]:%d\n", count);

		/* ������֡�е����ݰ��������������buffer */
		int i = 0;
		for (i = 0; i < MAX_PKT; i++)
			buffer.data[i] = (r.info.data)[i];

		/* �����ݰ����ݸ������ */
		to_network_layer(&buffer);
		printf("[Datalink layer send]:%d\n", count);
		count++;
		if (count > FILECOUNT)
			break;
	}
	while (1)
		sleep(1);
	exit(0);
}

//���������������
void physical_layer(char* ip_addr,int port)
{
	/* �޸Ľ����������ڸ��ݽ��������pidֵ�������ź� */
	char new_name[20] = "recv1_physic";
	prctl(PR_SET_NAME, new_name);

	struct sockaddr_in remote_addr; //�ͻ��������ַ�ṹ��
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;				  //ͨ��Э��Ϊ������
	remote_addr.sin_port = htons(port);	  //�������˿ں�
	remote_addr.sin_addr.s_addr = inet_addr(ip_addr); //������IP��ַ
	/*�����ͻ����׽���*/
	if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(-2);
	}

	/*���׽��ְ󶨵��ͻ��������ַ��*/
	if (connect(client_sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("connect error");
		exit(-1);
	}

	/* ��׽����㷢���ġ�д���źź��ٶ�ȡ���ݣ�����������·�㴫�� */
	(void)signal(SIG_WR, physical_write);

	while (1)
		sleep(1);
	close(client_sockfd);
	exit(0);
}

int main(int argc, char *argv[])
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
		else if (dl_pid > 0)
		{
			while ((ps_pid = fork()) < 0)
				sleep(1);

			if (ps_pid == 0)
			{
				//������fork�ӽ���--�����
				physical_layer(argv[1],atoi(argv[2]));
			}
			else if (ps_pid > 0) //������
				exit(0);
		}
	}
}