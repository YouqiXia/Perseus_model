import os
import subprocess
import concurrent.futures
import argparse

def get_files_from_dir(directory):
    return [os.path.join(directory, f) for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))]

def run_model(model, json_file, trace_file):
    log_file = os.path.splitext(os.path.basename(trace_file))[0] + ".log"
    json_name = os.path.splitext(os.path.basename(json_file))[0]
    
    folder_name = f"log/{json_name}"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
    command = f"{model} --json {json_file} --workload {trace_file} 2>&1 | tee ./log/{json_name}/{log_file}"
    
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    return

def main():
    parser = argparse.ArgumentParser(description = "parser")

    parser.add_argument('model', nargs='?', type=str, help='the model file')
    parser.add_argument('--config', '-c', type=str, help='the config directory')
    parser.add_argument('--trace', '-t', type=str, help='the trace directory')

    args = parser.parse_args()
    
    trace_files = get_files_from_dir(args.trace)
    json_files = get_files_from_dir(args.config)
    
    with concurrent.futures.ThreadPoolExecutor(max_workers = 4) as executor:
        futures = []
        for trace_file in trace_files:
            for json_file in json_files:
                futures = [executor.submit(run_model, args.model, json_file, trace_file)]
        
        for future in concurrent.futures.as_completed(futures):
            future.result()

if __name__ == "__main__":
    main()