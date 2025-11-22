#include <linux/swap.h>
#include <linux/module.h>
#include <trace/hooks/binder.h>
#include <uapi/linux/android/binder.h>
#include <uapi/linux/sched/types.h>
#include <linux/sched/prio.h>
#include <../../android/binder_internal.h>
#include <../../../kernel/sched/sched.h>
#include <linux/string.h> 

static const char *task_name[] = {
    "transsion.XOSLauncher", // com.transsion.xoslauncher
    "system_server",
    "surfaceflinger",	// android.systemui
    "cameraserver",
    "ndroid.systemui",  // com.android.systemui
	"droid.launcher3",	// com.android.launcher3
    "trebuchet",         // com.google.android.trebuchet
	"s.nexuslauncher",	// com.google.android.apps.nexuslauncher
	"droid.launcher",	// com.android.launcher
	"coilsw.launcher",	// com.teslacoilsw.launcher
	"putmethod.latin",	// com.google.android.inputmethod.latin, com.android.inputmethod.latin
	"chtype.swiftkey"	// com.touchtype.swiftkey
};

static const char *RenderThread = "RenderThread";
static const char *passBlur = "passBlur";
static const char *cameraserver_C3Dev = "C3Dev-";
static const char *cameraserver_ReqQ = "-ReqQ";

// Determine the status of the initiating task
// 1. Is it a listed important process?
// 2. Whether the binder initiated by this task is non-oneway
static bool set_binder_rt_task(struct binder_transaction *t)
{
    int i;
    if (!t || !t->from || !t->from->task || !t->to_proc ||
        !t->to_proc->tsk) {
        return false;
    }

    if (t->flags & TF_ONE_WAY) {
        return false;
    }

    if ((strcmp(t->from->task->group_leader->comm, task_name[0]) == 0) &&
        (strcmp(t->from->task->comm, RenderThread) == 0) &&
        (strcmp(t->to_proc->tsk->comm, task_name[2]) == 0)) {
        return true;
    }

    if ((strcmp(t->from->task->group_leader->comm, task_name[2]) == 0) &&
        (strcmp(t->from->task->comm, passBlur) == 0)) {
        return true;
    }

    if ((strcmp(t->from->task->group_leader->comm, task_name[3]) == 0) &&
        (strstr(t->from->task->comm, cameraserver_C3Dev) != NULL) &&
        (strstr(t->from->task->comm, cameraserver_ReqQ) != NULL)) {
        return true;
    }

    for (i = 0; i < sizeof(task_name) / sizeof(task_name[0]); i++) {
        if (strcmp(t->from->task->group_leader->comm, task_name[i]) == 0) {
            return true;
        }
    }
    return false;
}

static void extend_surfacefinger_binder_set_priority_handler(
    void *data, struct binder_transaction *t, struct task_struct *task)
{
    struct sched_param params;
    unsigned int policy;

    if (task && task->group_leader) {
		int i;
        for (i = 0; i < sizeof(task_name) / sizeof(task_name[0]); i++) {
            if (strcmp(task->group_leader->comm, task_name[i]) == 0) {
                policy = SCHED_RR;
                params.sched_priority = 15;

                if (rt_policy(policy)) {
                    sched_setscheduler_nocheck(task, policy | SCHED_RESET_ON_FORK,
                                   &params);
                    return;
                }
            }
        }
    }

    if (set_binder_rt_task(t)) {
        policy = SCHED_RR;
        params.sched_priority = 15;

        if (rt_policy(policy) && t->from && t->from->task) {
             sched_setscheduler_nocheck(t->from->task, policy | SCHED_RESET_ON_FORK, &params);
        }
    }
}

int __init binder_prio_init(void)
{
    int ret;

    pr_info("binder_prio: module init!");
    // extend_surfacefinger_binder_set_priority_handler 函数对应
    // trace_android_vh_binder_set_priority hook 点
    ret = register_trace_android_vh_binder_set_priority(
        extend_surfacefinger_binder_set_priority_handler, NULL);
    if (ret) {
        pr_err("binder_prio: failed to register set_priority handler, ret = %d\n", ret);
        return ret;
    }
    return 0;
}

void __exit binder_prio_exit(void)
{
    unregister_trace_android_vh_binder_set_priority(
        extend_surfacefinger_binder_set_priority_handler, NULL);

    pr_info("binder_prio: module exit!");
}

module_init(binder_prio_init);
module_exit(binder_prio_exit);
MODULE_LICENSE("GPL");
