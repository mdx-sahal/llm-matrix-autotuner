#### **AI-Driven Performance Optimization for Matrix Calculations**



**1. Executive Summary**

This project answers a fundamental software engineering question: \*Can an AI model act as a systems performance engineer and optimize low-level code entirely on its own?\*



To test this, we built a closed-loop system where an AI agent was tasked with speeding up a heavy mathematical calculation (multiplying two large $1024 \\times 1024$ grids of double-precision numbers) written in the C programming language. The system runs in a continuous, automated loop:

1\. \*\*Act:\*\* The system compiles and runs the C program using specific tuning settings.

2\. \*\*Observe:\*\* It measures exactly how fast the program ran and how efficiently it used the computer's memory hardware.

3\. \*\*Reason:\*\* An AI model looks at those raw results, figures out why the code was fast or slow, changes the settings, and pushes them to the next iteration.



We tested this automated tuning loop across two completely different machine environments: a local ASUS laptop and a cloud-hosted virtual machine. Without any human intervention, the AI successfully sped up the program by \*\*11.9x\*\* on the laptop and \*\*3.53x\*\* on the cloud server.



\---



**2. The Tuning Knobs**

The AI was given control over two critical parameters that dictate how modern processors handle heavy workloads:

\*   \*\*Thread Count:\*\* How many CPU cores work on the problem at the same time (parallelism via OpenMP).

\*   \*\*Tile Size:\*\* Instead of chewing through the massive grid of numbers all at once, the code breaks it into smaller, bite-sized blocks called "tiles." This keeps the active data footprint small enough to fit inside the CPU's ultra-fast internal temporary memory (the Cache), preventing the processor from constantly stalling to fetch data from the slower main system RAM.



\---



**3. Machine 1: Local ASUS Laptop (AMD Ryzen 7 HS)**

Our first machine is a high-performance laptop running on dedicated physical hardware. Because we had direct access to the physical hardware registers via Linux's `perf` utility, the AI could look at deep telemetry—specifically tracking the percentage of times the processor looked for data in its ultra-fast L1 Data Cache and missed.



\#### The AI's Learning Trajectory:

\*   \*\*Start (Baseline - Tile 256, Threads 1):\*\* 5.7974 seconds | L1 Cache Miss Rate: 64.36%

&#x20;   \*   \*AI Action:\* Recognized heavy compute time on a single core; shifted focus to scaling threads.

\*   \*\*Iteration 1 (Tile 128, Threads 1):\*\* 4.4826 seconds | L1 Cache Miss Rate: 65.15% 

&#x20;   \*   \*AI Action:\* Adjusted tile size but saw minor gains; decided to scale core utilization.

\*   \*\*Iteration 2 (Tile 32, Threads 4):\*\* 0.7167 seconds | L1 Cache Miss Rate: 59.33%

&#x20;   \*   \*AI Action:\* Multiplied threads while aggressively shrinking tile size to fit within L1 hardware constraints. Time dropped sharply.

\*   \*\*Iteration 3 (Tile 64, Threads 4):\*\* 0.9430 seconds | L1 Cache Miss Rate: 66.11%

&#x20;   \*   \*AI Action:\* Tried a slightly larger tile block, but performance regressed. Learned that smaller blocks are optimal.

\*   \*\*Final Best Settings (Tile 32, Threads 8):\*\* \*\*0.4871 seconds\*\* | L1 Cache Miss Rate: 62.98%



**Core Hardware Insight:**

The deep hardware telemetry revealed a critical limitation: even with the best tile configurations, the processor missed its memory cache about 60% of the time. This exposed a fundamental algorithmic ceiling in our C code. Because standard matrix multiplication requires accessing the second matrix column-by-column across a row-major memory mapping, it forces the computer to jump around frantically in memory. The AI successfully optimized the knobs to their absolute physical limits, but hit an unyielding wall built into the layout of the code itself.

\*   \*\*Local Speedup:\*\* \*\*11.9x\*\* over baseline.



\---



**4. Machine 2: Cloud Instance (Google Collab Shared Server)**

The second machine represents a shared, virtualized cloud environment. Because it runs inside a restricted virtualization layer, the deep hardware memory counters were blocked for security reasons. This forced the AI (powered here by a cloud-based Gemini model) to adapt and reason strictly using execution timers.



\#### The AI's Learning Trajectory:

\*   \*\*Start (Baseline - Tile 256, Threads 1):\*\* 3.6457 seconds

&#x20;   \*   \*AI Action:\* Demonstrated a faster initial single-thread speed than the laptop due to massive server L3 caches. The AI immediately scaled up threads to 8.

\*   \*\*Iteration 1 (Tile 64, Threads 8):\*\* 1.2181 seconds

&#x20;   \*   \*AI Action:\* Drastically dropped tile size and pushed core utilization. Time dropped significantly.

\*   \*\*Iteration 2 (Tile 32, Threads 8):\*\* 1.2172 seconds

&#x20;   \*   \*AI Action:\* Incremental gain recognized; attempted to minimize tile footprint further.

\*   \*\*Iteration 3 (Tile 32, Threads 16):\*\* 1.2579 seconds

&#x20;   \*   \*AI Action:\* Aggressively attempted to max out threads to 16, resulting in a performance regression due to hardware limits.

\*   \*\*Final Best Settings (Tile 16, Threads 8):\*\* \*\*1.0319 seconds\*\*



**Core Hardware Insight:**

While the cloud instance started with a faster baseline, its multi-threaded scaling hit a sharp wall. Pushing threads from 8 to 16 caused performance to degrade (1.21s to 1.25s). This proved that our allocated cloud virtual machine was physically capped at 8 logical processors; trying to force 16 threads introduced heavy operating system context-switching overhead. The AI successfully recognized this penalty, backed off to 8 threads, and tightly restricted the tile size to 16x16 to squeeze out the best performance.

\*   \*\*Cloud Speedup:\*\* \*\*3.53x\*\* over baseline.



\---



**5. Final Standings: How Our AI Tuner Compares to the Library**

To evaluate how good our autonomous tuner actually is, we compared our best versions against \*\*OpenBLAS\*\* (`cblas\_dgemm`)—an industry-standard, hand-optimized mathematical library used by professional engineers.



| Testing Environment     | Our AI-Tuned Time | OpenBLAS Pro Library Time | The Performance Gap          |

|-------------------------|-------------------|---------------------------|------------------------------|

| \*\*ASUS Laptop (Local)\*\* | 0.4871 seconds    | 0.0141 seconds            | OpenBLAS is \*\*34.5x Faster\*\* |

| \*\*Cloud Server (VM)\*\*   | 1.0319 seconds    | 0.0723 seconds            | OpenBLAS is \*\*14.2x Faster\*\* |



\---



**6. External API \& Cost Trade-Off Analysis**

To power the agent's reasoning engine, this project leveraged two distinct LLM services to optimize execution cost and network reliability across different hardware environments:



\*   \*\*Local Workstation (Ollama with Llama 3.1):\*\* Chosen for the local bare-metal tuning loop to ensure zero API latency, complete data privacy, and absolutely \*\*$0 execution cost\*\*. Running the model locally completely eliminated network bandwidth concerns during rapid-fire parameter testing.

\*   \*\*Cloud Virtual Machine (Google Gemini 2.5 Flash API):\*\* When moving to the network-isolated cloud environment, routing requests back to the local LLM via public SSH tunnels proved brittle, resulting in frequent 502 Bad Gateway timeouts. We pivoted to the Gemini 2.5 Flash API via the `google-genai` SDK. It was chosen because it provides a robust free tier that comfortably absorbed the tuning loop's token volume, ensuring ultra-fast, reliable reasoning without incurring runaway per-request cloud API costs.



\---



**7. Conclusion**

This project successfully demonstrates that an LLM agent, when given a strict feedback loop of performance data, can act as an automated systems engineer. It successfully mapped the unique performance profiles of both a local laptop and a cloud server, adjusted its strategy based on the architectural constraints of each machine, and maximized the relative performance of a naive C codebase.

