# Interplay between Topology and Edge Weights in Real-World Graphs: Concepts, Patterns, and an Algorithm

Source code for the paper **Interplay between Topology and Edge Weights in Real-World Graphs: Concepts, Patterns, and an Algorithm**, 
where we study the problem of generating realistic edge weights that are realistic in a macroscopic way to a given topology. 
For the problem, we explore the relations between edge weights and topology in real-world graphs and observe several pervasive patterns,
and propose **PEAR** (<ins><strong>P</strong></ins>attern-based <ins><strong>E</strong></ins>dge-weight <ins><strong>A</strong></ins>ssignment on g<ins><strong>R</strong></ins>aphs),
an algorithm using only two parameters for assigning realistic edge weights to a given topology by preserving all the observed macroscopic patterns.
Through extensive experiments on eleven real-world graphs, we show that PEAR generates more realistic edge weights than the baseline methods, {including some requiring much more prior knowledge and parameters.

Note: if a preview of the supplementary materials PDF file does not appear properly, please download the file.

## Requirements
All Python packages required for this repo are included in the *requirements* text file. 
Please run the following to install them.

    pip install -r requirements.txt
 

## Datasets

The raw datasets can be found in the *data_raw* folder.
In our experiments, we extract the largest connected component from each dataset.

Source:

- OF (OF): [http://opsahl.co.uk/tnet/datasets/OF_one-mode_weightedmsg_sum.txt](http://opsahl.co.uk/tnet/datasets/OF_one-mode_weightedmsg_sum.txt)
- openflights (FL): [http://opsahl.co.uk/tnet/datasets/openflights.txt](http://opsahl.co.uk/tnet/datasets/openflights.txt)
- threads-ask-ubuntu (th-UB): [https://www.cs.cornell.edu/~arb/data/threads-ask-ubuntu/](https://www.cs.cornell.edu/~arb/data/threads-ask-ubuntu/)
- threads-math-sx (th-MA): [https://www.cs.cornell.edu/~arb/data/threads-math-sx/](https://www.cs.cornell.edu/~arb/data/threads-math-sx/)
- threads-stack-overflow (th-SO): [https://www.cs.cornell.edu/~arb/data/threads-stack-overflow/](https://www.cs.cornell.edu/~arb/data/threads-stack-overflow/)
- sx-askubuntu (sx-UB): [https://snap.stanford.edu/data/sx-askubuntu.html](https://snap.stanford.edu/data/sx-askubuntu.html)
- sx-mathoverflow (sx-MA): [https://snap.stanford.edu/data/sx-mathoverflow.html](https://snap.stanford.edu/data/sx-mathoverflow.html)
- sx-stackoverflow (sx-SO): [https://snap.stanford.edu/data/sx-stackoverflow.html](https://snap.stanford.edu/data/sx-stackoverflow.html)
- sx-superuser (sx-SU): [https://snap.stanford.edu/data/sx-superuser.html](https://snap.stanford.edu/data/sx-superuser.html)
- coauth-DBLP (co-DB): [https://www.cs.cornell.edu/~arb/data/coauth-DBLP/](https://www.cs.cornell.edu/~arb/data/coauth-DBLP/)
- coauth-MAG-Geology (co-GE): [https://www.cs.cornell.edu/~arb/data/coauth-MAG-Geology/](https://www.cs.cornell.edu/~arb/data/coauth-MAG-Geology/) 

## Code

The code consists of the following parts, where each part is covered by a jupyter notebook: 

- [0.] data preprocessing @ *0-preprocessing.ipynb*
- [1.] observations @ *1-observations.ipynb* 
- [2.] experiments @ *2-experiments.ipynb*
 

### 0. data preprocessing
In this part, we conduct data preprocessing, mainly computing the metrics that we need later for the observations and the experiments.
In detail, it contains:

- reading the raw data and extracting the largest connected component
- computing the number of pairs sharing each specific number c of common neighbors (CNs)
- making the layers for each dataset and computing the number of pairs sharing each specific number c of CNs in the layers
- computing the metrics of edges


### 1. observations
In this part, we make our observations on the real-world dataset.
In detail, it contains:

- Why the number of common neighbors? Computing the Pearson correlation coefficient between the binary indicators of repetition (weight > 1) and the sequence of each edge-metric 
- Observation 1: adjacency and strongness. Computing the fractions of adjacent pairs and strong edges within each group of pairs/edges sharing c CNs for each number c of CNs and plotting the figures
- Observation 2: the fractions of strong edges. Computing the fraction of strong edges within each group of edges sharing c CNs for each number c of CNs and plotting the figures
- Observation 3: a power law across layers. Computing the fractions of strong edges of the edges without CNs and those of all edges for different layers and plotting the figures

### 2. algorithms experiments
In this part, we implement the proposed method and the baseline methods,
and perform the experiments to evaluate the methods. 
In detail, it contains:

- implementing each method
- analyzing the generated edge weights, especially computing the metrics that measure the distance between the edge weights and the ground-truth ones from different perspectives

### citation

    @article{Bu2023interplay,
      title={Interplay between topology and edge weights in real-world graphs: concepts, patterns, and an algorithm},
      author={Bu, Fanchen and Kang, Shinhwan and Shin, Kijung},
      journal={Data Mining and Knowledge Discovery},
      year={2023},
      volume={37},
      pages={2139 - 2191},
      publisher={Springer}
    }
