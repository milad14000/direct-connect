all: main

main: topology.o generator.o direct.o bipartite.o sim_util.o simulator.o b4.o vl2.o iewf.o log.o tdm.o dfs.o util.o bllb.o ps.o hedera.o
		g++ topology.o generator.o direct.o bipartite.o sim_util.o simulator.o b4.o vl2.o log.o tdm.o dfs.o util.o bllb.o ps.o hedera.o -o main

sim_util.o: sim_util.cc
		g++ -c sim_util.cc

bipartite.o: bipartite.cc
		g++ -c bipartite.cc

generator.o: generator.cc
		g++ -c generator.cc

direct.o: direct.cc
		g++ -c direct.cc

topology.o: topology.cc
		g++ -c topology.cc

simulator.o: simulator.cc
		g++ -c simulator.cc

tdm.o: tdm.cc
		g++ -c tdm.cc

log.o: log.cc
		g++ -c log.cc

b4.o: b4.cc
		g++ -c b4.cc

vl2.o: vl2.cc
		g++ -c vl2.cc

dfs.o: dfs.cc
		g++ -c dfs.cc

util.o: util.cc
		g++ -c util.cc

bllb.o: bllb.cc
		g++ -c bllb.cc

ps.o: ps.cc
		g++ -c ps.cc

hedera.o: hedera.cc
		g++ -c hedera.cc

clean:
		rm -rf *o main
