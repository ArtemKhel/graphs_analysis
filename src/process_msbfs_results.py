import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_dataset_groups_with_pairs(df, output='msbfs_grouped_pairs.png', normalize_by_nstart=False):
    # Identify GB and SPLA labels heuristically
    algos = df['algo'].unique()
    gb_label = None
    spla_label = None
    for a in algos:
        ua = a.upper()
        if 'GB' in ua and gb_label is None:
            gb_label = a
        if ('SPLA' in ua or 'PYSPARK' in ua or 'SPARK' in ua) and spla_label is None:
            spla_label = a
    # Fallback: pick first two if heuristics fail
    if gb_label is None or spla_label is None:
        uniq = list(algos)
        if len(uniq) >= 2:
            gb_label = uniq[0]
            spla_label = uniq[1]
        else:
            gb_label = uniq[0]
            spla_label = uniq[0]

    datasets = sorted(df['dataset'].unique())
    n_start = sorted(df['n_start_vert'].unique())

    # Layout parameters
    pair_width = 0.12
    gap_between_pairs = 0.05
    pairs_per_dataset = len(n_start)
    dataset_gap = 0.3
    group_width = pairs_per_dataset * (2 * pair_width + gap_between_pairs)

    # Build positions
    x_centers = []
    gb_means = []
    gb_stds = []
    spla_means = []
    spla_stds = []
    n_start_labels = []
    for d_idx, dataset in enumerate(datasets):
        base = d_idx * (group_width + dataset_gap)
        for j, n in enumerate(n_start):
            pair_base = base + j * (2 * pair_width + gap_between_pairs)
            # GB
            vals_gb = df[(df['dataset'] == dataset) & (df['n_start_vert'] == n) & (df['algo'] == gb_label)]['time']
            if normalize_by_nstart:
                vals_gb = vals_gb / n if not vals_gb.empty else vals_gb
            mean_gb = vals_gb.mean() if not vals_gb.empty else np.nan
            std_gb = vals_gb.std() if not vals_gb.empty else 0
            # SPLA
            vals_spla = df[(df['dataset'] == dataset) & (df['n_start_vert'] == n) & (df['algo'] == spla_label)]['time']
            if normalize_by_nstart:
                vals_spla = vals_spla / n if not vals_spla.empty else vals_spla
            mean_spla = vals_spla.mean() if not vals_spla.empty else np.nan
            std_spla = vals_spla.std() if not vals_spla.empty else 0

            x_centers.append(pair_base + pair_width)
            gb_means.append(mean_gb)
            gb_stds.append(std_gb)
            spla_means.append(mean_spla)
            spla_stds.append(std_spla)
            n_start_labels.append(n)

    # X tick positions for dataset labels: center of group
    dataset_ticks = []
    for d_idx, dataset in enumerate(datasets):
        base = d_idx * (group_width + dataset_gap)
        center = base + (pairs_per_dataset * (2 * pair_width + gap_between_pairs)) / 2
        dataset_ticks.append(center)

    # Now plot
    fig, ax = plt.subplots(figsize=(max(10, len(datasets) * 1.5), 6))
    # Plot GB and SPLA as offset from pair center
    n_pairs = len(x_centers)
    gb_pos = np.array(x_centers) - pair_width/2
    spla_pos = np.array(x_centers) + pair_width/2
    ax.bar(gb_pos, gb_means, width=pair_width, label=gb_label, yerr=gb_stds, capsize=3)
    ax.bar(spla_pos, spla_means, width=pair_width, label=spla_label, yerr=spla_stds, capsize=3)

    ax.set_yscale('log')
    ax.set_xticks(dataset_ticks)
    ax.set_xticklabels(datasets, rotation=0, ha='right')
    ax.set_xlabel('Dataset (groups)')
    if normalize_by_nstart:
        ax.set_ylabel('Time per start vertex (s, log scale)')
        ax.set_title('MSBFS: for each dataset, pairs (GB, SPLA) per n_start (normalized)')
    else:
        ax.set_ylabel('Time (s, log scale)')
        ax.set_title('MSBFS: for each dataset, pairs (GB, SPLA) per n_start')
    ax.legend()
    # Add secondary x-axis for n_start ticks above each group
    secax = ax.twiny()
    secax.set_xlim(ax.get_xlim())
    secax.set_xticks(x_centers)
    secax.set_xticklabels(n_start_labels, rotation=90, ha='center')
    secax.set_xlabel('n_start (per pair)', labelpad=10)
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
