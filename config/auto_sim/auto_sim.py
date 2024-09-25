import os
import subprocess
import concurrent.futures

# 获取 traces 目录下的所有文件
def get_trace_files(directory):
    return [os.path.join(directory, f) for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))]

# 执行 ./model 命令，接受 trace 文件作为参数
def run_model(trace_file):
    log_file = os.path.splitext(os.path.basename(trace_file))[0] + ".log"
    
    # 创建重定向的命令
    # command = f"../../cmake-build-debug/model --json ../../arches/config/perfect_ig_4issue_arch.json --workload {trace_file} 2>&1 | tee ./log/perfect_ig_4issue_arch/{log_file}"
    # command = f"../../cmake-build-debug/model --json ../../arches/config/load4_ig_4issue_arch.json --workload {trace_file} 2>&1 | tee ./log/load4_ig_4issue_arch/{log_file}"
    command = f"../../cmake-build-debug/model --json ../../arches/config/load4_ig_4issue_arch_pmuoff.json --workload {trace_file} 2>&1 | tee ./log/load4_ig_4issue_arch_pmuoff/{log_file}"
    
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    return

def main():
    # 指定 traces 目录
    traces_directory = "../../../traces"
    
    # 获取所有 trace 文件
    trace_files = get_trace_files(traces_directory)
    
    # 使用多线程执行程序
    with concurrent.futures.ThreadPoolExecutor() as executor:
        # 为每个 trace 文件启动一个线程
        futures = [executor.submit(run_model, trace_file) for trace_file in trace_files]
        
        # 等待所有线程完成
        for future in concurrent.futures.as_completed(futures):
            future.result()  # 处理线程的结果

if __name__ == "__main__":
    main()