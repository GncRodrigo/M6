
#include <bits/stdc++.h>
using namespace std;

enum TipoEvento { CHEGADA_Q1, FIM_S1, FIM_S2 };

struct Evento {
    double tempo;
    TipoEvento tipo;
    int id;
    Evento(double t, TipoEvento ty, int i): tempo(t), tipo(ty), id(i) {}
};

struct ComparadorEvento {
    bool operator()(Evento const& a, Evento const& b) const {
        if (a.tempo != b.tempo) return a.tempo > b.tempo;
        return a.id > b.id;
    }
};

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);


    const int c1 = 2, K1 = 3;                // Servidores e capacidade total da fila1
    const double chegada_baixa = 1.0, chegada_alta = 4.0;
    const double srv1_baixo = 3.0, srv1_alto = 4.0;

    const int c2 = 1, K2 = 5;                // Servidores e capacidade total da fila2
    const double srv2_baixo = 2.0, srv2_alto = 3.0;

    const double primeira_chegada = 1.5;

    long long N_chegadas_externas = 100000;
    if (argc >= 2) {
        try { N_chegadas_externas = stoll(argv[1]); } catch(...) { N_chegadas_externas = 100000; }
    }

    mt19937_64 gerador(42);
    uniform_real_distribution<double> uni01(0.0,1.0);
    auto sorteia_uniforme = [&](double a, double b){ return a + (b-a)*uni01(gerador); };

    double t = 0.0;
    long long geradas_externas = 0;

    // Separação entre clientes em serviço e clientes na fila
    int servico_q1 = 0, fila_q1 = 0;
    int servico_q2 = 0, fila_q2 = 0;

    long long perdas_q1 = 0, perdas_q2 = 0, atendidos_q2 = 0;

    vector<double> tempo_em_estado_q1(K1+1, 0.0);
    vector<double> tempo_em_estado_q2(K2+1, 0.0);

    priority_queue<Evento, vector<Evento>, ComparadorEvento> pq;
    int contador_id_evento = 0;
    auto proximo_id = [&](){ return ++contador_id_evento; };

    pq.emplace(primeira_chegada, CHEGADA_Q1, proximo_id());

    auto total_q1 = [&](){ return servico_q1 + fila_q1; };
    auto total_q2 = [&](){ return servico_q2 + fila_q2; };

    double tempo_final = 0.0;

    while (!pq.empty()) {
        Evento ev = pq.top(); pq.pop();
        double ev_tempo = ev.tempo;
        if (ev_tempo < t) ev_tempo = t;
        double dt = ev_tempo - t;
        if (dt < 0) dt = 0.0;

        // Atualiza tempo em estado
        if (total_q1() >=0 && total_q1() <= K1) tempo_em_estado_q1[total_q1()] += dt;
        if (total_q2() >=0 && total_q2() <= K2) tempo_em_estado_q2[total_q2()] += dt;
        t = ev_tempo;

        if (ev.tipo == CHEGADA_Q1) {
            if (total_q1() >= K1) {
                perdas_q1++; // fila cheia -> perda
            } else {
                if (servico_q1 < c1) {
                    servico_q1++;
                    double srv = sorteia_uniforme(srv1_baixo, srv1_alto);
                    pq.emplace(t + srv, FIM_S1, proximo_id());
                } else {
                    fila_q1++;
                }
            }

            // Agenda próxima chegada externa
            geradas_externas++;
            if (geradas_externas < N_chegadas_externas) {
                double ia = sorteia_uniforme(chegada_baixa, chegada_alta);
                pq.emplace(t + ia, CHEGADA_Q1, proximo_id());
            }
        }
        else if (ev.tipo == FIM_S1) {
            if (servico_q1 <= 0) continue;
            servico_q1--;

            // Envia para Q2
            if (total_q2() >= K2) perdas_q2++;
            else {
                if (servico_q2 < c2) {
                    servico_q2++;
                    double srv = sorteia_uniforme(srv2_baixo, srv2_alto);
                    pq.emplace(t + srv, FIM_S2, proximo_id());
                } else {
                    fila_q2++;
                }
            }

            if (fila_q1 > 0) {
                fila_q1--;
                servico_q1++;
                double srv = sorteia_uniforme(srv1_baixo, srv1_alto);
                pq.emplace(t + srv, FIM_S1, proximo_id());
            }
        }
        else if (ev.tipo == FIM_S2) {
            if (servico_q2 <= 0) continue;
            servico_q2--;
            atendidos_q2++;

            if (fila_q2 > 0) {
                fila_q2--;
                servico_q2++;
                double srv = sorteia_uniforme(srv2_baixo, srv2_alto);
                pq.emplace(t + srv, FIM_S2, proximo_id());
            }
        }

        tempo_final = t;
    }

    double tempo_total_sim = t;
    vector<double> prob_q1(K1+1, 0.0), prob_q2(K2+1, 0.0);
    if (tempo_total_sim > 0.0) {
        for (size_t i=0;i<tempo_em_estado_q1.size();++i) prob_q1[i] = tempo_em_estado_q1[i]/tempo_total_sim;
        for (size_t i=0;i<tempo_em_estado_q2.size();++i) prob_q2[i] = tempo_em_estado_q2[i]/tempo_total_sim;
    }
    cout.setf(std::ios::fixed);
    cout << setprecision(4);
    
    cout << "===== RESULTADOS DA SIMULAÇÃO =====\n\n";
    
    // Fila 1
    cout << "Fila 1:\n";
    cout << "Tempo final da simulação: " << tempo_total_sim << "\n";
    cout << "Clientes perdidos: " << perdas_q1 << "\n";
    cout << "Distribuição de estados (aprox.):\n";
    for (size_t i = 0; i < tempo_em_estado_q1.size(); ++i) {
        cout << "Estado " << i << ": " << (tempo_em_estado_q1[i]/tempo_total_sim) << "\n";
    }
    cout << "\n";
    
    // Fila 2
    cout << "Fila 2:\n";
    cout << "Tempo final da simulação: " << tempo_total_sim << "\n";
    cout << "Clientes perdidos: " << perdas_q2 << "\n";
    cout << "Distribuição de estados (aprox.):\n";
    for (size_t i = 0; i < tempo_em_estado_q2.size(); ++i) {
        cout << "Estado " << i << ": " << (tempo_em_estado_q2[i]/tempo_total_sim) << "\n";
    }
    cout << "\n";
    
    cout << "Total de clientes atendidos na fila 2: " << atendidos_q2 << "\n";
    

    return 0;
}
