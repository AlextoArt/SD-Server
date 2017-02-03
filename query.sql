SELECT U.Login, T.StatusId, T.Name, T.Path, T.Description FROM [Task] AS T
	INNER JOIN [ExecutorGroupExecutor] AS EG ON T.ExecutorGroupId = EG.ExecutorGroupId
	INNER JOIN [User] AS U ON EG.UserId = U.Id OR T.Executors LIKE CONCAT('%', U.Login, '%')
	WHERE T.StatusId NOT IN (28, 29, 30);