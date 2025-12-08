import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import math

def plot_category(df, category, algos, output):
    # Filter by category
    cat_df = df[df['algo'].str.contains(category)]
    datasets = cat_df['dataset'].unique()
    x = np.arange(len(datasets))
    width = 0.2
    fig, ax = plt.subplots(figsize=(12, 6))
    for i, algo in enumerate(algos):
        algo_df = cat_df[cat_df['algo'] == algo]
        means = []
        stds = []
        for dataset in datasets:
            vals = algo_df[algo_df['dataset'] == dataset]['time_of_iter'].astype(float)
            means.append(vals.mean() if not vals.empty else np.nan)
            stds.append(vals.std() if not vals.empty else 0)
        ax.bar(x + i*width, means, width, label=algo, yerr=stds, capsize=5)
    ax.set_xticks(x + width * (len(algos)-1)/2)
    ax.set_xticklabels(datasets, rotation=0, ha='right')
    ax.set_yscale('log')
    ax.set_ylabel('Time (s, log scale)')
    ax.set_title(f'{category} Triangle Counting Benchmark')
    ax.legend()
    plt.tight_layout()
    plt.savefig(output)
    plt.close()

def plot_grouped_by_algo_lib(df, output):
    # Compute mean and std times for each (algo, dataset)
    df['time_of_iter'] = df['time_of_iter'].astype(float)
    stats_df = df.groupby(['algo', 'dataset'])['time_of_iter'].agg(['mean', 'std']).reset_index()
    datasets = stats_df['dataset'].unique()
    # Compute overall mean for sorting
    dataset_means = stats_df.groupby('dataset')['mean'].mean().sort_values()
    sorted_datasets = dataset_means.index.tolist()
    # Prepare data for plotting
    burkhardt_gb = []
    burkhardt_gb_std = []
    burkhardt_spla = []
    burkhardt_spla_std = []
    sandia_gb = []
    sandia_gb_std = []
    sandia_spla = []
    sandia_spla_std = []
    for dataset in sorted_datasets:
        gb_b = stats_df[(stats_df['algo'] == 'GB_Burkhardt') & (stats_df['dataset'] == dataset)]
        spla_b = stats_df[(stats_df['algo'] == 'SPLA_Burkhardt') & (stats_df['dataset'] == dataset)]
        gb_s = stats_df[(stats_df['algo'] == 'GB_Sandia') & (stats_df['dataset'] == dataset)]
        spla_s = stats_df[(stats_df['algo'] == 'SPLA_Sandia') & (stats_df['dataset'] == dataset)]
        burkhardt_gb.append(gb_b['mean'].values[0] if not gb_b.empty else np.nan)
        burkhardt_gb_std.append(gb_b['std'].values[0] if not gb_b.empty else 0)
        burkhardt_spla.append(spla_b['mean'].values[0] if not spla_b.empty else np.nan)
        burkhardt_spla_std.append(spla_b['std'].values[0] if not spla_b.empty else 0)
        sandia_gb.append(gb_s['mean'].values[0] if not gb_s.empty else np.nan)
        sandia_gb_std.append(gb_s['std'].values[0] if not gb_s.empty else 0)
        sandia_spla.append(spla_s['mean'].values[0] if not spla_s.empty else np.nan)
        sandia_spla_std.append(spla_s['std'].values[0] if not spla_s.empty else 0)
    x = np.arange(len(sorted_datasets))
    width = 0.18
    fig, ax = plt.subplots(figsize=(18, 8))
    # For each dataset, plot Burkhardt (GB, SPLA) and Sandia (GB, SPLA) as pairs
    ax.bar(x - width*1.5, burkhardt_gb, width, label='GB_Burkhardt', yerr=burkhardt_gb_std, capsize=5, color='#1f77b4')
    ax.bar(x - width*0.5, burkhardt_spla, width, label='SPLA_Burkhardt', yerr=burkhardt_spla_std, capsize=5, color='#aec7e8')
    ax.bar(x + width*0.5, sandia_gb, width, label='GB_Sandia', yerr=sandia_gb_std, capsize=5, color='#ff7f0e')
    ax.bar(x + width*1.5, sandia_spla, width, label='SPLA_Sandia', yerr=sandia_spla_std, capsize=5, color='#ffbb78')
    ax.set_xticks(x)
    ax.set_xticklabels(sorted_datasets, rotation=0, ha='right')
    ax.set_yscale('log')
    ax.set_ylabel('Time (s, log scale)')
    ax.set_title('Burkhardt & Sandia (GB/SPLA) per Dataset')
    ax.legend()
    plt.tight_layout()
    plt.savefig(output)
    plt.close()

def main():
    df = pd.read_csv('./results.csv')
    burkhardt_algos = ['GB_Burkhardt', 'SPLA_Burkhardt']
    sandia_algos = ['GB_Sandia', 'SPLA_Sandia']
    plot_category(df, 'Burkhardt', burkhardt_algos, 'burkhardt_bar.png')
    plot_category(df, 'Sandia', sandia_algos, 'sandia_bar.png')
    plot_grouped_by_algo_lib(df, 'grouped_by_algo_lib.png')

if __name__ == "__main__":
    main()
