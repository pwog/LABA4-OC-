#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fdpub, fdpriv, fd;
	struct message msg; /*структура сообщения, которое читается из общедоступного именованного канала*/
	int n;
	char line[LINESIZE];
	loop: /* бесконечный цикл */
		/* Общедоступный канал открывается для чтения. 
		Системный вызов open() блокируется, если никакой 
		процесс-клиент не открывает другой конец канала*/
		if ((fdpub=open(PUBLIC, O_RDONLY)) == -1){ 
			perror (PUBLIC);
			exit(1);
		}

		/* Из общедоступного канала читается сообщение, 
		содержащее имена личного канала и требуемого файла. 
		Требуемый файл открывается для чтения. */
		while(read(fdpub, (char*)&msg, sizeof(msg)) > 0){
			if ((fd=open(msg.filename, O_RDONLY)) == -1){ 
				perror(msg.filename);
				break;
			}

			/* Личный канал открывается для записи. Файловый сервер спит, 
			пока клиент не откроет свой конец личного канала. 
			Если возникли проблемы, и клиент не может открыть канал, 
			сервер будет блокирован на этом open(). 
			!см. соответствующие ключи, предотвращающие блокировки. */
			if ((fdpriv=open(msg.privfifo, O_WRONLY)) == -1){ 
				perror(msg.privfifo);
				break;
			}

			/* данные из файла копируются в личный именованный канал */
			while((n = read(fd, line, LINESIZE)) > 0)
			{
			  int l = 0;
		    while(l<n)
		    {
		    	
			    int len = 0, len1 = 0;
		  		char *p = "";
					//Поиск длины строки
					p=(line + l);
					while(*p && *p!='\n')
					{
						++len;
						*p++; 
					}

					len1 = len;
					if (len>30)
						len1 = 30;

				  char str2[31] = "";
      		strncpy(str2, line+l, len1+1);
      		
				 	if (len>30)
				  	str2[len1] = '\n';

					write(fdpriv, str2, len1+1);
					l = l + len + 1;
				}
			}
			/* Когда копирование закончено, требуемый файл и 
			личный именованный канал закрывается */
			close (fd);
			close (fdpriv);
		}		
		/* end of first while() */
		/* Закрыть общедоступный канал. Сервер переходит 
		к следующему витку цикла. Причина для закрытия и 
		повторного открытия состоит в том, что хочется, 
		чтобы файловый сервер спал на open(), ожидая запросов. */
		close (fdpub);
		goto loop;
	/* !Программа никогда не завершается */
}