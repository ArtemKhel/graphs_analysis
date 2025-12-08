import sys
from pyspark.sql import SparkSession
from pyspark import StorageLevel
from pyspark.sql import functions as F
from pyspark.sql.types import StructType, StructField, LongType, IntegerType
import os
import time
import csv
import random


class MSBFS:
    def __init__(self, app_name="MS-BFS", master="local[*]"):
        self.spark = SparkSession.builder \
            .appName("MS-BFS") \
            .master("local[*]") \
            .config("spark.serializer", "org.apache.spark.serializer.KryoSerializer") \
            .config("spark.jars.packages", "graphframes:graphframes:0.8.2-spark3.1-s_2.12") \
            .config("spark.driver.memory", "4g") \
            .getOrCreate()
        self.spark.sparkContext.setCheckpointDir("/tmp/graph_ckpt")
        self.edges = None

    def stop(self):
        self.spark.stop()

    def load_from_file(self, path):
        """
        Загрузка ребёр из текстового файла: два целых номера на строку.
        """
        df = self.spark.read.text(path) \
            .select(F.split(F.col("value"), r"\s+").alias("cols")) \
            .filter(F.size("cols") == 2) \
            .select(
                F.col("cols").getItem(0).cast("long").alias("src"),
                F.col("cols").getItem(1).cast("long").alias("dst")
            )
        rev = df.select(F.col("dst").alias("src"), F.col("src").alias("dst"))
        edges = df.union(rev).distinct() \
            .repartition(200, "src") \
            .persist(StorageLevel.MEMORY_ONLY)
        self.edges = edges

    def run(self, sources):
        schema = StructType([
            StructField("vertex", IntegerType(), nullable=False),
            StructField("src",    IntegerType(), nullable=False),
            StructField("dist",   IntegerType(), nullable=False),
            StructField("parent", IntegerType(), nullable=True)
        ])

        data = [(int(s), int(s), 0, None) for s in sources]

        src_df = self.spark.createDataFrame(data, schema=schema)
        frontier = src_df.persist(StorageLevel.MEMORY_ONLY)
        visited = src_df.persist(StorageLevel.MEMORY_ONLY)

        frontier = frontier.checkpoint()
        visited = visited.checkpoint()

        while True:
            nbrs = frontier.alias("f") \
                .join(self.edges.alias("e"), F.col("f.vertex") == F.col("e.src")) \
                .select(
                    F.col("e.dst").alias("vertex"),
                    F.col("f.src"),
                    (F.col("f.dist") + 1).alias("dist"),
                    F.col("f.vertex").alias("parent")
                ) \
                .distinct()

            new_front = nbrs.join(
                visited.select("vertex", "src"),
                on=["vertex", "src"],
                how="left_anti"
            ).persist(StorageLevel.MEMORY_ONLY)

            if new_front.rdd.isEmpty():
                break

            visited = visited.union(new_front).persist(StorageLevel.MEMORY_ONLY)
            frontier = new_front

            visited = visited.checkpoint()

        return visited

def bench(data_dir, num_iters, out_csv='pyspark_msbfs_bench.csv'):
    random.seed(42)
    n_start_list = [1, 4, 16, 64, 256]
    datasets = [f for f in os.listdir(data_dir) if f.endswith('.txt')]
    with open(out_csv, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['PYSPARK_MSBFS', 'dataset', 'n_start_vert', 'time'])
        for dataset in datasets:
            path = os.path.join(data_dir, dataset)
            msbfs = MSBFS()
            msbfs.load_from_file(path)
            # Get all vertices
            all_vertices = msbfs.edges.select('src').union(msbfs.edges.select('dst')).distinct().rdd.map(lambda row: row[0]).collect()
            n = len(all_vertices)
            for n_start in n_start_list:
                for _ in range(num_iters):
                    if n_start > n:
                        continue
                    starts = random.sample(all_vertices, n_start)
                    t0 = time.time()
                    msbfs.run(starts)
                    t1 = time.time()
                    writer.writerow(['PYSPARK_MSBFS', dataset, n_start, t1-t0])
            msbfs.stop()

def main():
    if len(sys.argv) < 3:
        print("Usage: ms_bfs.py <edge_file> <start1>[,<start2>,...]")
        sys.exit(1)

    path = sys.argv[1]
    sources = sys.argv[2].split(",")

    bfs = MSBFS()
    bfs.load_from_file(path)
    result = bfs.run(sources)

    result.orderBy("src", "dist").show(20, truncate=False)
    print(f"Total reached: {result.count()}")

    bfs.stop()

if __name__ == "__main__":
    if len(sys.argv) == 3 and sys.argv[1] != '--help':
        data_dir = sys.argv[1]
        num_iters = int(sys.argv[2])
        bench(data_dir, num_iters)
    else:
        main()