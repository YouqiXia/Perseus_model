import os
import subprocess
import concurrent.futures
import argparse

def get_files_from_dir(directory):
    return [os.path.join(directory, f) for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))]

def run_model(model, json_file, trace_file, log_path):
    log_file = os.path.splitext(os.path.basename(trace_file))[0] + ".log"
    json_name = os.path.splitext(os.path.basename(json_file))[0]
    
    folder_name = f"./log/{log_path}"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
        
    folder_name = f"./log/{log_path}/{json_name}"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
        
    command = f"{model} --json {json_file} --workload {trace_file} 2>&1 | tee ./log/{log_path}/{json_name}/{log_file}"
    
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    return

def find_and_save_lines_with_marker(folder_path, marker, output_file):
    for root, dirs, files in os.walk(folder_path):
        if not dirs: 
            result_lines = []
            for file_name in files:
                if file_name.endswith(".log"):
                    file_path = os.path.join(root, file_name)
                    try:
                        with open(file_path, "r") as f:
                            lines = f.readlines()
                            for i, line in enumerate(lines):
                                if "model" in line.strip() and marker in lines[i - 1].strip() and i > 1:
                                    statistics = line.strip()
                                    result_lines.append(f"File: {file_name},\n{statistics}")
                    except Exception as e:
                        print(f"Error processing file {file_path}: {e}")
            with open(f"{root}/{output_file}", "w") as f:
                for result in result_lines:
                    f.write(result + "\n")
                print(f"Processed completed. Results saved to {root}/{output_file}")

def main():
    parser = argparse.ArgumentParser(description = "parser")

    parser.add_argument('model', nargs='?', type=str, help='the model file')
    parser.add_argument('--config', '-c', type=str, help='the config directory')
    parser.add_argument('--trace', '-t', type=str, help='the trace directory')
    parser.add_argument('--no-run', action='store_true', help='Flag to skip running the process')

    args = parser.parse_args()
    
    trace_files = get_files_from_dir(args.trace)
    json_files = get_files_from_dir(args.config)
    
    folder_name = "./log"
    if not os.path.exists(folder_name):
        os.mkdir(folder_name)
    log_path = os.path.basename(args.config)
    
    if not args.no_run:
        with concurrent.futures.ThreadPoolExecutor(max_workers = 4) as executor:
            futures = []
            for trace_file in trace_files:
                for json_file in json_files:
                    futures = [executor.submit(run_model, args.model, json_file, trace_file, log_path)]
            
            for future in concurrent.futures.as_completed(futures):
                future.result()
    
    find_and_save_lines_with_marker(f"log/{log_path}", "=============================================================", "final.log")
    

if __name__ == "__main__":
    main()