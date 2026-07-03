import subprocess
import requests
import re
from google import genai

# Initialize the new SDK client with your API key
client = genai.Client(api_key="YOUR_API_KEY_HERE")

def run_c_engine(matrix_size, tile_size, threads):
    print(f"--> Running timing execution (Tile: {tile_size}, Threads: {threads})...")

    result = subprocess.run(
        ["./matrix_cache", str(matrix_size), str(tile_size), str(threads)],
        capture_output=True,
        text=True
    )

    try:
        output_parts = result.stdout.strip().split(',')
        custom_time = float(output_parts[0])
        blas_time = float(output_parts[1])
        is_correct = int(output_parts[2])

        if is_correct == 1:
            print(f"    Agent Time: {custom_time:.4f}s | OpenBLAS: {blas_time:.4f}s")
            return custom_time, blas_time
        else:
            print("    Math FAILED!")
            return None, None

    except Exception as e:
        print(f"    Error parsing C output: {e}")
        return None, None

def ask_llm_for_next_params(history_text):
    prompt = f"""
    You are an expert low-level performance engineer tuning a C matrix multiplication algorithm.
    The matrix size is 1024x1024.
    
    We are tuning TWO parameters:
    1. 'tile_size' (to optimize CPU Cache hits, usually 16 to 256)
    2. 'threads' (number of parallel CPU cores to use, usually 1 to 16)
    
    Here is the timing history of our experiments so far:
    {history_text}
    
    Analyze the data and suggest the next configuration to minimize execution time.
    Provide your final selected integers inside specific tags at the very end.
    
    Example format:
    <tile>64</tile>
    <threads>8</threads>
    """
    
    print("\n[Asking Cloud LLM...]")
    
    try:
        response = client.models.generate_content(
            model='gemini-2.5-flash',
            contents=prompt
        )
        raw_answer = response.text
        
        tile_match = re.search(r'<tile>(\d+)</tile>', raw_answer)
        thread_match = re.search(r'<threads>(\d+)</threads>', raw_answer)
        
        next_tile = int(tile_match.group(1)) if tile_match else 32
        next_threads = int(thread_match.group(1)) if thread_match else 4
        
        return next_tile, next_threads
            
    except Exception as e:
        print(f"--> Error communicating with Cloud LLM: {e}")
        return 32, 4

def main():
    matrix_size = 1024
    history = []

    print("\nStarting Cloud Notebook Tuning Loop...\n")

    curr_tile, curr_threads = 256, 1
    custom_t, blas_t = run_c_engine(matrix_size, curr_tile, curr_threads)
    if custom_t:
        history.append((curr_tile, curr_threads, custom_t))

    for iteration in range(4):
        print(f"\n--- Iteration {iteration + 1} ---")

        history_text = ""
        for ts, th, t in history:
            history_text += f"- Tile Size {ts}, Threads {th}: {t:.4f} seconds\n"

        curr_tile, curr_threads = ask_llm_for_next_params(history_text)
        custom_t, blas_t = run_c_engine(matrix_size, curr_tile, curr_threads)

        if custom_t:
            history.append((curr_tile, curr_threads, custom_t))

    print("\n--- Tuning Complete ---")
    if not history:
        print("No successful runs recorded.")
        return

    best_run = min(history, key=lambda x: x[2])
    print(f"Best Configuration: Tile = {best_run[0]}, Threads = {best_run[1]}")
    print(f"Best Execution Time: {best_run[2]:.4f} seconds")

if __name__ == "__main__":
    main()