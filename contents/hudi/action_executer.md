ActionExecutor组织形式
=====================
* BaseCommitActionExecutor
  * CommitActionExecutor 
    * InsertCommitActionExecutor 核心execute调用了 WriteHelper.write(..., performTagging = false)
    * UpsertCommitActionExecutor 核心execute调用了 WriteHelper.write(..., performTagging = true)

  * DeltaCommitActionExecutor
    * InsertDeltaCommitActionExecutor 核心execute调用了 WriteHelper.write(..., performTagging = false)
    * UpsertDeltaCommitActionExecutor 核心execute调用了 WriteHelper.write(..., performTagging = false)