import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_dataset_groups_with_pairs(df, output='msbfs_grouped_pairs.png', normalize_by_nstart=False):
    # Identify all unique algos and sort for consistent color/legend
    algos = sorted(df['algo'].unique())
    datasets = sorted(df['dataset'].unique())
    n_start = sorted(df['n_start_vert'].unique())

    # Layout parameters
    bar_width = 0.12
    gap_between_bars = 0.0
    bars_per_pair = len(algos)
    pairs_per_dataset = len(n_start)
    group_width = pairs_per_dataset * (bars_per_pair * bar_width + (bars_per_pair-1)*gap_between_bars)
    dataset_gap = 0.5

    # Build positions and values
    bar_positions = []
    bar_means = []
    bar_stds = []
    bar_labels = []
    n_start_labels = []
    x_centers = []
    for d_idx, dataset in enumerate(datasets):
        base = d_idx * (group_width + dataset_gap)
        for j, n in enumerate(n_start):
            pair_base = base + j * (bars_per_pair * bar_width + (bars_per_pair-1)*gap_between_bars)
            for a_idx, algo in enumerate(algos):
                vals = df[(df['dataset'] == dataset) & (df['n_start_vert'] == n) & (df['algo'] == algo)]['time']
                if normalize_by_nstart:
                    vals = vals / n if not vals.empty else vals
                mean = vals.mean() if not vals.empty else np.nan
                std = vals.std() if not vals.empty else 0
                bar_pos = pair_base + a_idx * (bar_width + gap_between_bars)
                bar_positions.append(bar_pos)
                bar_means.append(mean)
                bar_stds.append(std)
                bar_labels.append(algo)
            # For secondary x-axis
            x_centers.append(pair_base + (bars_per_pair * bar_width + (bars_per_pair-1)*gap_between_bars)/2)
            n_start_labels.append(n)

    # X tick positions for dataset labels: center of group
    dataset_ticks = []
    for d_idx, dataset in enumerate(datasets):
        base = d_idx * (group_width + dataset_gap)
        center = base + (pairs_per_dataset * (bars_per_pair * bar_width + (bars_per_pair-1)*gap_between_bars)) / 2
        dataset_ticks.append(center)

    # Now plot
    fig, ax = plt.subplots(figsize=(max(10, len(datasets) * 1.5), 6))
    # Plot bars for all algos for each n_start in each dataset group
    unique_labels = list(dict.fromkeys(bar_labels))
    colors = plt.cm.get_cmap('tab10', len(unique_labels))
    color_map = {lbl: colors(i) for i, lbl in enumerate(unique_labels)}
    for lbl in unique_labels:
        idxs = [i for i, l in enumerate(bar_labels) if l == lbl]
        ax.bar([bar_positions[i] for i in idxs], [bar_means[i] for i in idxs], width=bar_width,
               label=lbl, yerr=[bar_stds[i] for i in idxs], capsize=3, color=color_map[lbl])

    ax.set_yscale('log')
    ax.set_xticks(dataset_ticks)
    ax.set_xticklabels(datasets, rotation=0, ha='right')
    ax.set_xlabel('Dataset (groups)')
    if normalize_by_nstart:
        ax.set_ylabel('Time per start vertex (s, log scale)')
        ax.set_title('MSBFS: for each dataset, bars per n_start (normalized)')
    else:
        ax.set_ylabel('Time (s, log scale)')
        ax.set_title('MSBFS: for each dataset, bars per n_start')
    ax.legend()
    # Add secondary x-axis for n_start ticks above each group
    secax = ax.twiny()
    secax.set_xlim(ax.get_xlim())
    secax.set_xticks(x_centers)
    secax.set_xticklabels(n_start_labels, rotation=90, ha='center')
    secax.set_xlabel('n_start (per group)', labelpad=10)
    plt.tight_layout()
    plt.savefig(output)
    plt.close()

# Integrate into main run
def main():
    df = pd.read_csv('msbfs_bench.csv')
    plot_dataset_groups_with_pairs(df)
    plot_dataset_groups_with_pairs(df, output='msbfs_grouped_pairs_norm.png', normalize_by_nstart=True)

if __name__ == "__main__":
    main()
