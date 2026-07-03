# AI-Driven Matrix Multiplication Auto-Tuner

## Overview
This project features an automated, LLM-driven tuning loop that optimizes a C-based matrix multiplication engine by dynamically adjusting thread counts and tile sizes based on performance telemetry.

## Dependencies
*   **C Compiler:** `gcc` with OpenMP support.
*   **Libraries:** `libopenblas-dev` for standard library benchmarking.
*   **Python 3.x**
*   **Python Packages:** `requests`, `google-genai`
*   **LLM Engine:** A valid Google Gemini API Key.

## Setup & Execution
1. Install system dependencies:
   `sudo apt-get update && sudo apt-get install libopenblas-dev linux-tools-common linux-tools-generic`
2. Install Python dependencies:
   `pip install requests google-genai`
3. **Important:** If running the cloud variation, insert your Gemini API key into the `client = genai.Client(api_key="YOUR_API_KEY_HERE")` variable in the Python script.
4. Run the automated agent:
   `python3 agent.py`