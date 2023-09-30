import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes

# Načti CSV soubor
df = pd.read_csv('results.csv')

df_aggregated = df.groupby(["query_count", "num_urls"]).mean().reset_index()

# Seznam algoritmů
algorithms = ['ModuloString', 'ModuloString_view', 'CompareString', 'CompareString_view']

cz = {}
cz["axe_y"] = "Čas zpracovaní v microsekundách"
cz["axe_x"] = "Počet zpracovaných URL"
cz["query_count"] = "Počet query"
cz["query_count_label"] = "Funkce"
cz["sub_plots"] = "subplots_CZ"
cz["zoom_plots"] = "porovnani_CZ"

en = {}
en["axe_y"] = "Processing time in microseconds"
en["axe_x"] = "Number of processed URLs"
en["query_count"] = "Number of query"
en["query_count_label"] = "Functions"
en["sub_plots"] = "subplots_EN"
en["zoom_plots"] = "compare_EN"

languages = [cz, en]

for lang in languages:
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    axes = axes.flatten()

    for i, algorithm in enumerate(algorithms):
        ax = axes[i]
        ax.set_title(algorithm)
        ax.set_xlabel(lang["axe_x"])
        ax.set_ylabel(lang["axe_y"])

        for query_count in df_aggregated['query_count'].unique():
            df_filtered = df_aggregated[df_aggregated['query_count'] == query_count]
            ax.plot(df_filtered['num_urls'], df_filtered[algorithm], label=f'{lang["query_count"]}={query_count}')

        ax.legend()

    plt.tight_layout()
    plt.savefig(lang["sub_plots"] + '.png', dpi=300)
    plt.show()


    fig, ax1 = plt.subplots(figsize=(10, 6))

    for algorithm in algorithms:
        df_mean = df_aggregated.groupby('num_urls')[algorithm].mean().reset_index()
        ax1.plot(df_mean['num_urls'], df_mean[algorithm], label=algorithm)

    ax1.set_xlabel(lang["axe_x"])
    ax1.set_ylabel(lang["axe_y"])
    ax1.legend(title=lang["query_count_label"])

    ax2 = inset_axes(ax1, width="40%", height="40%", loc='lower right', bbox_to_anchor=(0, 0.05, 1, 1), bbox_transform=ax1.transAxes)

    for algorithm in algorithms:
        df_mean = df_aggregated.groupby('num_urls')[algorithm].mean().reset_index()
        ax2.plot(df_mean['num_urls'], df_mean[algorithm])

    ax2.set_xlim(2300, 2511)
    ax2.set_ylim(930, 1050)

    plt.savefig(lang["zoom_plots"] + '.png', dpi=300)
    plt.show()


