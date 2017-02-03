#ifndef TASK_TABLE_H_
#define TASK_TABLE_H_

#define RES_LOGIN_LENGTH	64
#define RES_STID_LENGTH		sizeof(int)
#define RES_NAME_LENGTH		64
#define RES_PATH_LENGTH		64
#define RES_DESC_LENGTH		128

struct TaskTable
{
	int stat;					//	StatusID
	char name[RES_NAME_LENGTH];	//	Task name
	char path[RES_PATH_LENGTH];	//	Task path
	char desc[RES_DESC_LENGTH];	//	Task description
};

#endif // TASK_TABLE_H_
