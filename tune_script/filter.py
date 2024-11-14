import re
from heapq import nlargest

top_entries = []

# Initialize variables to store maximum TFlops and the corresponding entry
max_tflops = 0
best_entry = ""

# Regular expression to match the TFlops and valid entry lines
perf_pattern = re.compile(r"Perf: .*?(\d+\.\d+) TFlops")
valid_pattern = re.compile(r"^\d+$")

with open("../tunning_output/int8_216064x4608x1152_mi300_2.txt", "r") as file:
    current_entry = ""
    for line in file:
        # Accumulate lines to form the current entry
        current_entry += line

        # Check if this line contains TFlops data
        perf_match = perf_pattern.search(line)
        if perf_match:
            # Parse the TFlops value
            tflops = float(perf_match.group(1))

        # Check if this line contains a valid/invalid entry (0 or 1)
        valid_match = valid_pattern.match(line.strip())
        if valid_match:
            valid = int(valid_match.group())
            # If valid (1), add the entry and TFlops to the list
            if valid == 1:
                top_entries.append((tflops, current_entry))
            
            # Reset current entry after finding a valid/invalid line
            current_entry = ""

# Get the top 5 entries by TFlops
top_5_entries = nlargest(5, top_entries, key=lambda x: x[0])

# Output the top 5 entries
print("Top 5 Entries with Max TFlops:")
for idx, (tflops, entry) in enumerate(top_5_entries, 1):
    print(f"Rank {idx} - TFlops: {tflops}")
    print(entry)
    print("=" * 50)