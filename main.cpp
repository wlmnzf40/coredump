#include <bthread/bthread.h>
#include <butil/macros.h>
#include <butil/synchronization/lock.h>
#include <vector>
#include <memory>
#include <unistd.h>
#include <signal.h>
#include <atomic>
#include <cassert>
#include "gflags/gflags.h"

// 节点结构体
struct node {
    int weight = 333;
    // 可根据需要添加其他成员
    node(int w) : weight(w) {}
};

class NodeLists {
public:
    void Add(node* n) {
        nodes.push_back(n);
        std::atomic_thread_fence(std::memory_order_release);
        physical_size.fetch_add(1, std::memory_order_release);  // 原子自增
    }

    // 获取起始迭代器
    auto cBegin() const {
        return nodes.cbegin();
    }

    // 获取结束迭代器（基于物理大小）
    auto cEnd() const {
        return cBegin() + physical_size.load(std::memory_order_acquire);
    }

    // 遍历函数示例
    void Traverse() {
        for (auto it = cBegin(); it != cEnd(); ++it) {
            assert((*it)->weight != 555);
        }
    }

    void Clear() {
        for (node* n : nodes) {
            delete n;
        }
        nodes.clear();
        physical_size.store(0, std::memory_order_release);
    }

    std::vector<node*> nodes;
private:
    std::atomic<int> physical_size{0};  // 原子计数器
};

class test {
public:
    // 成员函数实现
    void Get() {
        GetMultiConditionsNodeListsParallel();
        MergeNode();
    }

    void Clear() {
        pivot_attr_trees_.Clear();
    }

    void Traverse() {
        pivot_attr_trees_.Traverse();
    }

    void GetMultiConditionsNodeListsParallel() {
        // 创建3个bthread
        for (int i = 0; i < 3; ++i) {
            Param *p = new Param();
            p->nodes.reserve(3);
            _params.push_back(p);
        }
        for (int i = 0; i < 3; ++i) {
            bthread_t tid;

            if (bthread_start_background(&tid, NULL, ThreadFunc, _params[i]) != 0) {
                LOG(ERROR) << "Failed to create bthread";
                continue;
            }
            _tids.push_back(tid);
        }
    }

    void MergeNode() {
        // 等待所有线程完成
        for (auto& tid : _tids) {
            bthread_join(tid, NULL);
        }

        // 合并结果
        std::lock_guard<std::mutex> lock(pivot_attr_trees_mutex_);
        for (auto& param : _params) {
            for (node* n : param->nodes) {
                pivot_attr_trees_.Add(n);
            }
            break;
        }

        // 清理临时数据（索引0的nodes已转移，其余未合并的需手动释放）
        for (size_t i = 0; i < _params.size(); ++i) {
            if (i > 0) {
                for (node* n : _params[i]->nodes) {
                    delete n;
                }
            }
            delete _params[i];
        }
        _params.clear();
        _tids.clear();
    }

private:
    // 线程参数结构体
    struct Param {
        std::vector<node*> nodes;
    };

    // 线程函数
    static void* ThreadFunc(void* arg) {
        //std::unique_ptr<Param> p(static_cast<Param*>(arg));
        Param* p = static_cast<Param*>(arg);
        // 创建新节点（示例创建5个节点）
        for (int i = 0; i < 1; ++i) {
            node* n = new node(333); // 使用内存池更佳
            p->nodes.push_back(n);
        }

        return NULL;
    }

    NodeLists pivot_attr_trees_;
    std::vector<bthread_t> _tids;
    std::vector<Param*> _params;
    std::mutex pivot_attr_trees_mutex_;
};

volatile sig_atomic_t stop = 0;
void signal_handler(int signum) {
    stop = 1;
}

static void* mainThread(void* arg) {
    test tester;
    tester.Get();
    tester.Traverse();
    tester.Clear();
    return NULL;
}

static void* mainBThread(void* arg) {
    while (!stop) {
        std::vector<bthread_t> _tids;
        for (int i = 0; i < 200; ++i) {
            bthread_t tid;
            if (bthread_start_background(&tid, NULL, mainThread, NULL) != 0) {
                LOG(ERROR) << "Failed to create bthread";
                continue;
            }
            _tids.push_back(std::move(tid));
        }
        for (auto& tid : _tids) {
            bthread_join(tid, NULL);
        }

        usleep(100);  // 100ms延迟防止CPU跑满
    }
}

int main(int argc, char** argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    bthread_t tid;
    if (bthread_start_background(&tid, NULL, mainBThread, NULL) != 0) {
        LOG(ERROR) << "Failed to create bthread";
    }
    bthread_join(tid, NULL);

    return 0;
}
                                                 