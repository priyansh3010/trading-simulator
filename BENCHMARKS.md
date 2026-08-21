# Trading Simulator Benchmarks

Benchmark results for the trading order book simulator across multiple optimization versions. All benchmarks were run with 1,000,000 orders for latency measurements and 10,000,000 orders for throughput measurements. All test data were synthesized orders, with no extra operations.

**Test Environment:**
- CPU: Ryzen 5 4600H (TSC frequency: ~2.994 GHz)
- OS: Windows 11

## Version History

| Version | Description |
| :--- | :--- |
| v0 | Naive implementation (baseline) |
| v1 | Improved memory management |
| v2 | Custom intrusive DLL |
| v2.1 | Optimized access lines |
| v2.2 | Constant deletion times |
| v3 | Pre-allocated nodes and orders |
| v4 | Bids and asks as vectors |
| v5 | Removed dummy node creation |
| v6 | Merged node and order structs |

---

## Throughput (Orders Per Second)

Higher is better. Throughput measured as orders added per second.

| Version | Uniform | Exponential | Normal | Log Normal | Student T |
| :--- | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 24,028.2 | 40,130.6 | 3,886.57 | 4,149.69 | 1,139.19 |
| v1_improved_memory_management | 24,263.8 | 41,014.4 | 3,919.68 | 4,124.75 | 1,135.54 |
| v2_custom_instrusive_DLL | 2,742,780 | 2,638,110 | 2,704,480 | 2,714,910 | 2,870,310 |
| v2.1_optimized_access_lines | 3,358,390 | 3,255,560 | 3,233,950 | 3,217,910 | 3,292,280 |
| v2.2_constant_deletion_times | 3,359,620 | 3,247,540 | 3,261,490 | 3,264,410 | 3,288,410 |
| v3_pre_allocated_nodes_and_orders | 20,578,300 | 22,628,800 | 21,554,300 | 19,925,300 | 23,001,200 |
| v4_bids_and_asks_vectorr | 40,025,200 | 40,984,000 | 40,064,100 | 40,175,400 | 39,882,300 |
| v5_removed_dummy_nodes_creation | 39,522,900 | 41,108,200 | 39,965,600 | 40,677,400 | 39,345,500 |
| v6_merged_node_and_order_structs | 43,925,800 | 45,742,000 | 44,673,800 | 44,783,300 | 45,786,500 |

---

## Per-Order Latency (Nanoseconds)

Lower is better. All latencies measured in nanoseconds.

### Uniform Distribution

| Version | min | max | mean | p50 | p90 | p99 | p99.9 | p99.99 |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 50 | 41,794,555 | 1,888.12 | 621 | 5,019 | 9,567 | 22,341 | 43,651 |
| v1_improved_memory_management | 50 | 40,277,429 | 1,835.43 | 601 | 4,949 | 9,137 | 19,446 | 31,929 |
| v2_custom_instrusive_DLL | 30 | 46,414,443 | 361.64 | 270 | 530 | 801 | 21,289 | 37,539 |
| v2.1_optimized_access_lines | 30 | 45,130,146 | 309.244 | 190 | 420 | 681 | 21,429 | 38,251 |
| v2.2_constant_deletion_times | 30 | 44,920,575 | 306.03 | 190 | 410 | 671 | 21,670 | 37,880 |
| v3_pre_allocated_nodes_and_orders | 20 | 76,813 | 68.574 | 60 | 110 | 170 | 390 | 3,576 |
| v4_bids_and_asks_vectorr | 20 | 43,491 | 42.4401 | 40 | 50 | 130 | 360 | 571 |
| v5_removed_dummy_nodes_creation | 20 | 25,727 | 43.4267 | 40 | 50 | 130 | 360 | 561 |
| v6_merged_node_and_order_structs | 20 | 21,680 | 42.2972 | 40 | 50 | 130 | 350 | 711 |

### Exponential Distribution

| Version | min | max | mean | p50 | p90 | p99 | p99.9 | p99.99 |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 50 | 44,122,767 | 861.498 | 460 | 1,653 | 3,807 | 6,341 | 37,279 |
| v1_improved_memory_management | 50 | 43,649,086 | 870.574 | 460 | 1,653 | 3,867 | 5,941 | 35,656 |
| v2_custom_instrusive_DLL | 30 | 48,476,908 | 361.611 | 250 | 551 | 831 | 13,635 | 31,779 |
| v2.1_optimized_access_lines | 30 | 49,581,469 | 305.879 | 180 | 420 | 691 | 13,174 | 34,013 |
| v2.2_constant_deletion_times | 30 | 48,245,722 | 298.879 | 180 | 410 | 681 | 12,633 | 30,997 |
| v3_pre_allocated_nodes_and_orders | 20 | 82,113 | 63.9387 | 60 | 100 | 180 | 400 | 1,232 |
| v4_bids_and_asks_vectorr | 30 | 81,471 | 39.9082 | 30 | 50 | 130 | 340 | 1,041 |
| v5_removed_dummy_nodes_creation | 20 | 86,932 | 40.5737 | 40 | 50 | 130 | 330 | 551 |
| v6_merged_node_and_order_structs | 20 | 24,545 | 38.6793 | 30 | 50 | 130 | 330 | 671 |

### Normal Distribution

| Version | min | max | mean | p50 | p90 | p99 | p99.9 | p99.99 |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 70 | 45,500,472 | 7,015.39 | 1,102 | 24,115 | 34,895 | 41,537 | 55,884 |
| v1_improved_memory_management | 70 | 45,375,185 | 6,813.01 | 1,041 | 23,413 | 33,843 | 36,077 | 48,290 |
| v2_custom_instrusive_DLL | 30 | 48,065,000 | 359.774 | 230 | 530 | 831 | 15,589 | 32,881 |
| v2.1_optimized_access_lines | 30 | 47,336,942 | 301.147 | 170 | 410 | 681 | 16,731 | 32,019 |
| v2.2_constant_deletion_times | 20 | 47,403,282 | 302.604 | 170 | 410 | 691 | 17,282 | 31,779 |
| v3_pre_allocated_nodes_and_orders | 20 | 82,173 | 65.867 | 60 | 100 | 170 | 410 | 2,314 |
| v4_bids_and_asks_vectorr | 20 | 43,711 | 40.0201 | 40 | 50 | 120 | 280 | 631 |
| v5_removed_dummy_nodes_creation | 20 | 79,377 | 40.8997 | 40 | 50 | 120 | 260 | 530 |
| v6_merged_node_and_order_structs | 20 | 51,295 | 40.4149 | 40 | 50 | 130 | 300 | 591 |

### Log Normal Distribution

| Version | min | max | mean | p50 | p90 | p99 | p99.9 | p99.99 |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 100 | 43,947,128 | 7,161.84 | 1,112 | 24,535 | 34,083 | 39,093 | 70,632 |
| v1_improved_memory_management | 100 | 43,866,532 | 6,939.28 | 1,031 | 23,944 | 33,262 | 35,646 | 50,644 |
| v2_custom_instrusive_DLL | 30 | 47,950,506 | 364.186 | 230 | 541 | 841 | 14,617 | 34,454 |
| v2.1_optimized_access_lines | 30 | 47,673,470 | 299.444 | 160 | 400 | 691 | 13,645 | 30,737 |
| v2.2_constant_deletion_times | 30 | 47,178,904 | 298.1 | 170 | 400 | 671 | 13,194 | 30,597 |
| v3_pre_allocated_nodes_and_orders | 30 | 56,234 | 64.6583 | 60 | 100 | 160 | 370 | 1,102 |
| v4_bids_and_asks_vectorr | 30 | 34,564 | 40.0467 | 40 | 50 | 120 | 250 | 551 |
| v5_removed_dummy_nodes_creation | 30 | 79,598 | 40.4892 | 40 | 50 | 120 | 250 | 470 |
| v6_merged_node_and_order_structs | 20 | 49,151 | 39.6217 | 40 | 50 | 120 | 250 | 561 |

### Student T Distribution

| Version | min | max | mean | p50 | p90 | p99 | p99.9 | p99.99 |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| v0_naive_implementation | 120 | 44,791,967 | 19,647.8 | 1,442 | 63,648 | 80,119 | 114,383 | 195,044 |
| v1_improved_memory_management | 100 | 44,043,902 | 19,343.5 | 1,382 | 62,726 | 79,989 | 113,191 | 208,869 |
| v2_custom_instrusive_DLL | 30 | 47,046,030 | 342.987 | 210 | 500 | 811 | 15,158 | 34,344 |
| v2.1_optimized_access_lines | 30 | 47,560,189 | 299.112 | 160 | 400 | 681 | 13,254 | 30,837 |
| v2.2_constant_deletion_times | 20 | 47,530,209 | 296.686 | 160 | 390 | 681 | 14,366 | 32,039 |
| v3_pre_allocated_nodes_and_orders | 30 | 74,408 | 61.9782 | 50 | 90 | 160 | 380 | 1,102 |
| v4_bids_and_asks_vectorr | 20 | 16,170 | 38.3144 | 40 | 40 | 120 | 240 | 490 |
| v5_removed_dummy_nodes_creation | 20 | 15,268 | 39.2407 | 40 | 40 | 120 | 260 | 530 |
| v6_merged_node_and_order_structs | 20 | 17,101 | 38.4782 | 40 | 40 | 120 | 260 | 550 |
