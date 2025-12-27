import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import math

def plot_category(df, category, algos, output):
    # Filter by category
    cat_df = df[df['algo'].str.contains(category)]
    datasets = cat_df['dataset'].unique()
    x = np.arange(len(datasets))
    # Split algos into CPU and GPU groups (heuristic: presence of 'GPU' in name)
    cpu_algos = [a for a in algos if 'GPU' not in a.upper()]
    gpu_algos = [a for a in algos if 'GPU' in a.upper()]
    total_algos = cpu_algos + gpu_algos
    n_total = len(total_algos)
    width = 0.2
    total_width = n_total * width

    fig, ax = plt.subplots(figsize=(12, 6))
    for idx, algo in enumerate(total_algos):
        algo_df = cat_df[cat_df['algo'] == algo]
        means = []
        stds = []
        for dataset in datasets:
            vals = algo_df[algo_df['dataset'] == dataset]['time_of_iter'].astype(float)
            means.append(vals.mean() if not vals.empty else np.nan)
            stds.append(vals.std() if not vals.empty else 0)
        # position offset per dataset
        # start from left edge: x - total_width/2, then add idx*width
        offset = x - total_width/2 + idx * width
        ax.bar(offset, means, width, label=algo, yerr=stds, capsize=5)
    # center xticks under grouped bars
    ax.set_xticks(x)
    ax.set_xticklabels(datasets, rotation=0, ha='right')
    ax.set_yscale('log')
    ax.set_ylabel('Time (s, log scale)')
    ax.set_title(f'{category} Triangle Counting Benchmark')
    ax.legend()
    plt.tight_layout()
    plt.savefig(output)
    plt.close()

def plot_grouped_by_algo_lib(df, output, log_y=True):
    # Compute mean and std times for each (algo, dataset)
    df['time_of_iter'] = df['time_of_iter'].astype(float)
    stats_df = df.groupby(['algo', 'dataset'])['time_of_iter'].agg(['mean', 'std']).reset_index()
    datasets = stats_df['dataset'].unique()
    # Compute overall mean for sorting
    dataset_means = stats_df.groupby('dataset')['mean'].mean().sort_values()
    sorted_datasets = dataset_means.index.tolist()
    # Prepare data for plotting
    # Support GB, SPLA (CPU) and SPLAGPU variants for both Burkhardt and Sandia
    burkhardt_gb = []
    burkhardt_spla = []
    burkhardt_spla_std = []
    burkhardt_gb_std = []
    burkhardt_splagpu = []
    burkhardt_splagpu_std = []
    sandia_gb = []
    sandia_gb_std = []
    sandia_spla = []
    sandia_spla_std = []
    sandia_splagpu = []
    sandia_splagpu_std = []
    for dataset in sorted_datasets:
        gb_b = stats_df[(stats_df['algo'] == 'GB_Burkhardt') & (stats_df['dataset'] == dataset)]
        spla_b = stats_df[(stats_df['algo'] == 'SPLA_Burkhardt') & (stats_df['dataset'] == dataset)]
        splagpu_b = stats_df[(stats_df['algo'] == 'SPLAGPU_Burkhardt') & (stats_df['dataset'] == dataset)]
        gb_s = stats_df[(stats_df['algo'] == 'GB_Sandia') & (stats_df['dataset'] == dataset)]
        spla_s = stats_df[(stats_df['algo'] == 'SPLA_Sandia') & (stats_df['dataset'] == dataset)]
        splagpu_s = stats_df[(stats_df['algo'] == 'SPLAGPU_Sandia') & (stats_df['dataset'] == dataset)]
        burkhardt_gb.append(gb_b['mean'].values[0] if not gb_b.empty else np.nan)
        burkhardt_gb_std.append(gb_b['std'].values[0] if not gb_b.empty else 0)
        burkhardt_spla.append(spla_b['mean'].values[0] if not spla_b.empty else np.nan)
        burkhardt_spla_std.append(spla_b['std'].values[0] if not spla_b.empty else 0)
        burkhardt_splagpu.append(splagpu_b['mean'].values[0] if not splagpu_b.empty else np.nan)
        burkhardt_splagpu_std.append(splagpu_b['std'].values[0] if not splagpu_b.empty else 0)
        sandia_gb.append(gb_s['mean'].values[0] if not gb_s.empty else np.nan)
        sandia_gb_std.append(gb_s['std'].values[0] if not gb_s.empty else 0)
        sandia_spla.append(spla_s['mean'].values[0] if not spla_s.empty else np.nan)
        sandia_spla_std.append(spla_s['std'].values[0] if not spla_s.empty else 0)
        sandia_splagpu.append(splagpu_s['mean'].values[0] if not splagpu_s.empty else np.nan)
        sandia_splagpu_std.append(splagpu_s['std'].values[0] if not splagpu_s.empty else 0)
    x = np.arange(len(sorted_datasets))
    # four/three variants: GB, SPLA (CPU), SPLAGPU (GPU) per family
    width = 0.1
    fig, ax = plt.subplots(figsize=(20, 8))
    # For each dataset, plot Burkhardt (GB, SPLA, SPLAGPU) and Sandia (GB, SPLA, SPLAGPU)
    # Offsets: -1.5w, -0.5w, +0.5w for Burkhardt; +1.5w, +2.5w, +3.5w for Sandia (keeps spacing)
    ax.bar(x - width*3.5, burkhardt_gb, width, label='GB_Burkhardt', yerr=burkhardt_gb_std, capsize=5, color='#1f77b4')
    ax.bar(x - width*2.5, sandia_gb, width, label='GB_Sandia', yerr=sandia_gb_std, capsize=5, color='#aec7e8')
    ax.bar(x - width*0.5, burkhardt_spla, width, label='SPLA_Burkhardt', yerr=burkhardt_spla_std, capsize=5, color='#ff7f0e')
    ax.bar(x + width*0.5, sandia_spla, width, label='SPLA_Sandia', yerr=sandia_spla_std, capsize=5, color='#ffbb78')
    ax.bar(x + width*1.5, burkhardt_splagpu, width, label='SPLAGPU_Burkhardt', yerr=burkhardt_splagpu_std, capsize=5, color="#a02c2c")
    ax.bar(x + width*2.5, sandia_splagpu, width, label='SPLAGPU_Sandia', yerr=sandia_splagpu_std, capsize=5, color="#df8a8a")

    ax.set_xticks(x)
    ax.set_xticklabels(sorted_datasets, rotation=0, ha='right')
    ax.set_yscale('log' if log_y else 'linear')
    ax.set_ylabel('Time (s, log scale)' if log_y else 'Time (s)')
    ax.set_title('Burkhardt & Sandia (GB/SPLA) per Dataset')
    ax.legend()
    plt.tight_layout()
    plt.savefig(output)
    plt.close()

def main():
    df = pd.read_csv('./bench_tc.csv')
    burkhardt_algos = ['GB_Burkhardt', 'SPLA_Burkhardt', 'SPLAGPU_Burkhardt']
    sandia_algos = ['GB_Sandia', 'SPLA_Sandia', 'SPLAGPU_Sandia']
    plot_category(df, 'Burkhardt', burkhardt_algos, 'burkhardt_bar.png')
    plot_category(df, 'Sandia', sandia_algos, 'sandia_bar.png')
    plot_grouped_by_algo_lib(df, 'grouped_by_algo_lib.png')

if __name__ == "__main__":
    main()
