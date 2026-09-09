#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
import json
import sys
import argparse
import os

def process_file(input_path, bigger_is_better, smaller_is_better):
    with open(input_path, 'r') as f:
        data = json.load(f)
    
    for scenario in data.get('scenarios', []):
        name = scenario.get('name', 'Unknown')
        results = scenario.get('results', {})
        
        if 'fps_avg' in results:
            bigger_is_better.append({
                "name": f"{name} (Avg FPS)",
                "value": results['fps_avg'],
                "unit": "fps"
            })
            
        if 'frame_time_p99_ms' in results:
            smaller_is_better.append({
                "name": f"{name} (P99 Frame Time)",
                "value": results['frame_time_p99_ms'],
                "unit": "ms"
            })

        if 'frame_interval_p99_ms' in results:
            smaller_is_better.append({
                "name": f"{name} (P99 Frame Interval)",
                "value": results['frame_interval_p99_ms'],
                "unit": "ms"
            })
            
        if 'data_throughput_mpps' in results:
            bigger_is_better.append({
                "name": f"{name} (Throughput)",
                "value": results['data_throughput_mpps'],
                "unit": "Mpts/sec"
            })

def main():
    parser = argparse.ArgumentParser(description="Convert QAccelPlot benchmark JSON to github-action-benchmark custom JSON")
    parser.add_argument('inputs', nargs='+', help="Input JSON files")
    parser.add_argument('--bigger-is-better-output', required=True, help="Output JSON for metrics where larger values are better")
    parser.add_argument('--smaller-is-better-output', required=True, help="Output JSON for metrics where smaller values are better")
    args = parser.parse_args()
    
    bigger_is_better = []
    smaller_is_better = []
    for input_file in args.inputs:
        if os.path.exists(input_file):
            try:
                process_file(input_file, bigger_is_better, smaller_is_better)
            except Exception as e:
                print(f"Error processing {input_file}: {e}", file=sys.stderr)
        else:
            print(f"Warning: {input_file} not found.", file=sys.stderr)
            
    with open(args.bigger_is_better_output, 'w') as f:
        json.dump(bigger_is_better, f, indent=2)
    with open(args.smaller_is_better_output, 'w') as f:
        json.dump(smaller_is_better, f, indent=2)

if __name__ == '__main__':
    main()
