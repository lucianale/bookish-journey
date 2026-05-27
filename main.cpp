#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <sstream>
#include <stack>
#include <queue>

using namespace std;

struct Ruta {
    string aerolinea;
    int id_aerolinea;
    int id_origen;
    int id_destino;
    char codeshare;
};

struct Aeropuerto {
    int id;
    string nombre;
    string ciudad;
    string pais;
    string iata;
    double latitud;
    double longitud;
    vector<Ruta> rutas;
};

void leerAeropuertos(
    unordered_map<int, Aeropuerto>& aeropuertos,
    unordered_map<string, int>& codigosIATA,
    unordered_map<string, int>& nombresAeropuertos,
    const string nombreArchivo
)
{
    ifstream archivo(nombreArchivo.c_str());

    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir " << nombreArchivo << endl;
        return;
    }

    string linea;
    int lineasLeidas = 0;

    while (getline(archivo, linea)) {

        stringstream ss(linea);
        string campo;
        vector<string> campos;

        while (getline(ss, campo, ',')) {
            campos.push_back(campo);
        }
        try {
            Aeropuerto apt;
            apt.id = stoi(campos[0]);
            apt.nombre = campos[1];
            apt.ciudad = campos[2];
            apt.pais = campos[3];
            apt.iata = campos[4];
            apt.latitud = stod(campos[5]);
            apt.longitud = stod(campos[6]);

            aeropuertos[apt.id] = apt;

            if (apt.iata != "\\N" && !apt.iata.empty()) {
                codigosIATA[apt.iata] = apt.id;
            }
            if (!apt.nombre.empty()) {
                nombresAeropuertos[apt.nombre] = apt.id;
            }

            lineasLeidas++;

        }
        catch (const exception& e) {
            continue;
        }
    }

    archivo.close();

    cout << "----------LECTURA DE AEROPUERTOS----------" << endl;
    cout << "Archivo: " << nombreArchivo << endl;
    cout << "Aeropuertos cargados: " << lineasLeidas << endl;
    cout << "Total en mapa: " << aeropuertos.size() << endl;

    return;
}

void leerRutas(unordered_map<int, Aeropuerto>& aeropuertos, const string nombreArchivo) {
    ifstream archivo(nombreArchivo.c_str());
    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo de rutas: " << nombreArchivo << endl;
        return;
    }

    string linea;
    int rutasCargadas = 0;
    int rutasIgnoradasDuplicadas = 0;
    int rutasIgnoradasNulasOInexistentes = 0;

    unordered_set<string> conexionesRegistradas;

    while (getline(archivo, linea)) {
        if (linea.empty()) {
            continue;
        }

        stringstream ss(linea);
        string campo;
        vector<string> campos;

        while (getline(ss, campo, ',')) {
            campos.push_back(campo);
        }

        string aerolinea = campos[0];
        string aerolinea_id = campos[1];
        string origen_id = campos[3];
        string destino_id = campos[5];
        string codeshare = campos[6];

        if (origen_id == "\\N" || destino_id == "\\N" || origen_id.empty() || destino_id.empty()) {
            rutasIgnoradasNulasOInexistentes++;
            continue;
        }

        try {
            int id_origen = stoi(origen_id);
            int id_destino = stoi(destino_id);

            if (aeropuertos.find(id_origen) == aeropuertos.end() || aeropuertos.find(id_destino) == aeropuertos.end()) {
                rutasIgnoradasNulasOInexistentes++;
                continue;
            }

            string claveConexion = to_string(id_origen) + "-" + to_string(id_destino);
            if (conexionesRegistradas.find(claveConexion) != conexionesRegistradas.end()) {
                rutasIgnoradasDuplicadas++;
                continue;
            }

            int id_aerolinea = -1;
            if (aerolinea_id != "\\N" && !aerolinea_id.empty()) {
                id_aerolinea = stoi(aerolinea_id);
            }

            char codeshare_val = ' ';
            if (!codeshare.empty() && codeshare[0] == 'Y') {
                codeshare_val = 'Y';
            }

            Ruta r;
            r.aerolinea = aerolinea;
            r.id_aerolinea = id_aerolinea;
            r.id_origen = id_origen;
            r.id_destino = id_destino;
            r.codeshare = codeshare_val;

            aeropuertos[id_origen].rutas.push_back(r);

            conexionesRegistradas.insert(claveConexion);
            rutasCargadas++;
        }
        catch (const exception& e) {
            rutasIgnoradasNulasOInexistentes++;
            continue;
        }
    }

    archivo.close();

    cout << "----------LECTURA DE RUTAS----------" << endl;
    cout << "Archivo: " << nombreArchivo << endl;
    cout << "Rutas asignadas exitosamente al grafo: " << rutasCargadas << endl;
    cout << "Rutas descartadas (duplicadas origen-destino): " << rutasIgnoradasDuplicadas << endl;
    cout << "Rutas descartadas (IDs nulos o no existentes en aeropuertos): " << rutasIgnoradasNulasOInexistentes << endl;
}

void mostrarAeropuertoPorId(const unordered_map<int, Aeropuerto>& aeropuertos, int idBuscar) {
    auto it = aeropuertos.find(idBuscar);

    if (it != aeropuertos.end()) {
        const Aeropuerto& apt = it->second;

        cout << "-----------------INFORMACION DEL AEROPUERTO-----------------" << endl;;
        cout << "ID:        " << apt.id << endl;
        cout << "Nombre:    " << apt.nombre << endl;
        cout << "Ciudad:    " << apt.ciudad << endl;
        cout << "País:    " << apt.pais << endl;
        cout << "IATA:      " << apt.iata << endl;
        cout << "Ubicacion: Lat " << apt.latitud << " | Lon " << apt.longitud << endl;
        cout << "Rutas:     " << apt.rutas.size() << " registradas." << endl;
    }
    else {
        cout << "Error: El aeropuerto con ID " << idBuscar << " no existe." << endl;
    }
}

//Reto 1
int alcancePersonalizado(const unordered_map<int, Aeropuerto>& aeropuertos, int idOrigen) {
    queue<pair<int, int>> q;
    unordered_set<int> visitados;

    q.push({ idOrigen, 0 });
    visitados.insert(idOrigen);

    int contadorAeropuertosAlcanzados = 0;

    while (!q.empty()) {
        int idActual = q.front().first;
        int vuelosActuales = q.front().second;
        q.pop();

        if (vuelosActuales >= 4) {
            continue;
        }

        auto it_buscado = aeropuertos.find(idActual);

        if (it_buscado == aeropuertos.end()) {
            continue;
        }

        const Aeropuerto aeropuertoActual = it_buscado->second;

        for (const Ruta ruta : aeropuertoActual.rutas) {
            int idDestino = ruta.id_destino;

            if (visitados.find(idDestino) == visitados.end()) {
                visitados.insert(idDestino);

                q.push({ idDestino, vuelosActuales + 1 });

                contadorAeropuertosAlcanzados++;
            }
        }
    }

    return contadorAeropuertosAlcanzados;
}

void reto1(const unordered_map<string, int>& nombresAeropuertos, const unordered_map<string, int>& codigosIATA, const unordered_map<int, Aeropuerto>& aeropuertos) {
    cout << "RETO 1" << endl;
    string aeropuertoOrigen;
    getline(cin, aeropuertoOrigen);
    if (aeropuertoOrigen.length() > 3) {
        auto it_buscado = nombresAeropuertos.find(aeropuertoOrigen);

        if (it_buscado == nombresAeropuertos.end()) {
            cout << "El aeropuerto de origen no existe." << endl;
        }
        else {
            int aeropuertoId = it_buscado->second;
            cout << "Cantidad de aeropuertos a alcanzar (maximo 3 escalas): " << alcancePersonalizado(aeropuertos, aeropuertoId) << endl;
        }
    }
    else
    {
        auto it_buscado = codigosIATA.find(aeropuertoOrigen);

        if (it_buscado == codigosIATA.end()) {
            cout << "El aeropuerto de origen no existe." << endl;
        }
        else {
            int aeropuertoId = it_buscado->second;
            cout << "Cantidad de aeropuertos a alcanzar (máximo 3 escalas): " << alcancePersonalizado(aeropuertos, aeropuertoId) << endl;
        }
    }
}

//Reto 2
void dfs1(int nodo, unordered_map<int, Aeropuerto>& aeropuertos,
    unordered_set<int>& visitado, stack<int>& pila) {

    visitado.insert(nodo);

    for (Ruta& r : aeropuertos[nodo].rutas) {
        int vecino = r.id_destino;
        if (visitado.find(vecino) == visitado.end()) {
            dfs1(vecino, aeropuertos, visitado, pila);
        }
    }

    pila.push(nodo);
}

void dfs2(int nodo, unordered_map<int, Aeropuerto>& aeropuertos,
    unordered_set<int>& visitado, vector<int>& componente) {

    visitado.insert(nodo);
    componente.push_back(nodo);

    for (auto& par : aeropuertos) {
        for (Ruta& r : par.second.rutas) {
            if (r.id_destino == nodo && visitado.find(par.first) == visitado.end()) {
                dfs2(par.first, aeropuertos, visitado, componente);
            }
        }
    }
}

void reto2(unordered_map<int, Aeropuerto>& aeropuertos) {
    cout << "RETO 2" << endl;

    unordered_set<int> visitado;
    stack<int> pila;

    for (auto& par : aeropuertos) {
        int nodo = par.first;
        if (visitado.find(nodo) == visitado.end()) {
            dfs1(nodo, aeropuertos, visitado, pila);
        }
    }
    unordered_set<int> visitado2;
    vector<vector<int>> componentes;

    while (!pila.empty()) {
        int nodo = pila.top();
        pila.pop();

        if (visitado2.find(nodo) == visitado2.end()) {
            vector<int> componente;
            dfs2(nodo, aeropuertos, visitado2, componente);
            componentes.push_back(componente);
        }
    }
    int maxSize = 0;
    for (auto& comp : componentes) {
        if ((int)comp.size() > maxSize) {
            maxSize = comp.size();
        }
    }

    cout << "Total de grupos: " << componentes.size() << endl;
    cout << "Grupo mas grande: " << maxSize << " aeropuertos" << endl;
}

double calcularDistanciaGeo(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0;
    const double PI = 3.141592653589793;
    double phi1 = lat1 * PI / 180.0;
    double phi2 = lat2 * PI / 180.0;
    double dPhi = (lat2 - lat1) * PI / 180.0;
    double dLam = (lon2 - lon1) * PI / 180.0;
    double a = std::sin(dPhi / 2) * std::sin(dPhi / 2) +
        std::cos(phi1) * std::cos(phi2) *
        std::sin(dLam / 2) * std::sin(dLam / 2);
    return R * 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
}

//Reto 3
unordered_map<int, double> dijkstra(int origen, unordered_map<int, Aeropuerto>& aeropuertos) {
    unordered_map<int, double> distancia;

    for (auto& par : aeropuertos) {
        distancia[par.first] = DBL_MAX;
    }
    distancia[origen] = 0;

    priority_queue<pair<double, int>> pq;
    pq.push({ 0, origen });

    while (!pq.empty()) {
        double distActual = -pq.top().first;
        int actual = pq.top().second;
        pq.pop();

        if (distActual > distancia[actual]) continue;

        for (Ruta& r : aeropuertos[actual].rutas) {
            int vecino = r.id_destino;
            double peso = calcularDistanciaGeo(
                aeropuertos[actual].latitud, aeropuertos[actual].longitud,
                aeropuertos[vecino].latitud, aeropuertos[vecino].longitud
            );
            double nuevaDist = distActual + peso;

            if (nuevaDist < distancia[vecino]) {
                distancia[vecino] = nuevaDist;
                pq.push({ -nuevaDist, vecino });
            }
        }
    }
    return distancia;
}

void reto3(unordered_map<int, Aeropuerto>& aeropuertos) {
    cout << "RETO 3" << endl;

    double maxDistancia = 0;
    int aeropuerto1 = -1;
    int aeropuerto2 = -1;

    for (auto& par : aeropuertos) {
        int origen = par.first;
        unordered_map<int, double> distancias = dijkstra(origen, aeropuertos);

        for (auto& dest : distancias) {
            if (dest.second < DBL_MAX && dest.second > maxDistancia) {
                maxDistancia = dest.second;
                aeropuerto1 = origen;
                aeropuerto2 = dest.first;
            }
        }
    }

    cout << "Origen: " << aeropuertos[aeropuerto1].iata << endl;
    cout << "Destino: " << aeropuertos[aeropuerto2].iata << endl;
    cout << "Distancia: " << maxDistancia << " km" << endl;
}

//Reto 4
bool esSudamerica(const string pais) {
    if (pais == "Argentina") return true;
    if (pais == "Bolivia") return true;
    if (pais == "Brasil") return true;
    if (pais == "Chile") return true;
    if (pais == "Colombia") return true;
    if (pais == "Ecuador") return true;
    if (pais == "Guyana") return true;
    if (pais == "Paraguay") return true;
    if (pais == "Peru") return true;
    if (pais == "Suriname") return true;
    if (pais == "Uruguay") return true;
    if (pais == "Venezuela") return true;
    return false;
}

void reto4(unordered_map<int, Aeropuerto>& aeropuertos) {
    cout << "RETO 4" << endl;

    vector<int> idsSA;
    for (auto par : aeropuertos) {
        if (esSudamerica(par.second.pais)) {
            idsSA.push_back(par.first);
        }
    }

    unordered_set<int> visitados;
    priority_queue<pair<double, int>> pq;

    int inicio = idsSA[0];
    visitados.insert(inicio);

    for (Ruta r : aeropuertos[inicio].rutas) {
        if (esSudamerica(aeropuertos[r.id_destino].pais)) {
            double peso = calcularDistanciaGeo(
                aeropuertos[inicio].latitud, aeropuertos[inicio].longitud,
                aeropuertos[r.id_destino].latitud, aeropuertos[r.id_destino].longitud
            );
            pq.push({ -peso, r.id_destino });
        }
    }

    double totalKm = 0;
    int conectados = 1;

    while (!pq.empty() && conectados < (int)idsSA.size()) {
        double peso = -pq.top().first;
        int nodo = pq.top().second;
        pq.pop();

        if (visitados.find(nodo) != visitados.end()) continue;

        visitados.insert(nodo);
        totalKm += peso;
        conectados++;

        for (Ruta r : aeropuertos[nodo].rutas) {
            if (esSudamerica(aeropuertos[r.id_destino].pais) &&
                visitados.find(r.id_destino) == visitados.end()) {
                double pesoNuevo = calcularDistanciaGeo(
                    aeropuertos[nodo].latitud, aeropuertos[nodo].longitud,
                    aeropuertos[r.id_destino].latitud, aeropuertos[r.id_destino].longitud
                );
                pq.push({ -pesoNuevo, r.id_destino });
            }
        }
    }
    cout << "Aeropuertos en Sudamerica: " << idsSA.size() << endl;
    cout << "Conectados: " << conectados << endl;
    cout << "Kilometraje total: " << totalKm << " km" << endl;
}

//Reto 5
double dijkstraPorAerolinea(int origen, int destino, string aerolinea,
    unordered_map<int, Aeropuerto>& aeropuertos) {

    unordered_map<int, double> dist;
    for (auto p : aeropuertos) {
        dist[p.first] = DBL_MAX;
    }
    dist[origen] = 0;

    priority_queue<pair<double, int>> pq;
    pq.push({ 0, origen });

    while (!pq.empty()) {
        double actual = -pq.top().first;
        int nodo = pq.top().second;
        pq.pop();

        if (actual > dist[nodo]) continue;
        if (nodo == destino) return actual;

        for (Ruta r : aeropuertos[nodo].rutas) {
            if (r.aerolinea == aerolinea) {
                double peso = calcularDistanciaGeo(
                    aeropuertos[nodo].latitud, aeropuertos[nodo].longitud,
                    aeropuertos[r.id_destino].latitud, aeropuertos[r.id_destino].longitud
                );
                double nueva = actual + peso;
                if (nueva < dist[r.id_destino]) {
                    dist[r.id_destino] = nueva;
                    pq.push({ -nueva, r.id_destino });
                }
            }
        }
    }
    return -1;
}

void reto5(unordered_map<int, Aeropuerto>& aeropuertos, const unordered_map<string, int>& codigosIATA) {
    cout << "RETO 5" << endl;

    string orig, dest, aero;
    cout << "Origen IATA: "; cin >> orig;
    cout << "Destino IATA: "; cin >> dest;
    cout << "Aerolinea: "; cin >> aero;

    auto itOrg = codigosIATA.find(orig);
    auto itDest = codigosIATA.find(dest);
    if (itOrg == codigosIATA.end() || itDest == codigosIATA.end()) {
        cout << "Aeropuerto no encontrado" << endl;
        return;
    }
    int idOrg = itOrg->second;
    int idDest = itDest->second;

    double d = dijkstraPorAerolinea(idOrg, idDest, aero, aeropuertos);

    if (d >= 0) {
        cout << "Distancia: " << d << " km" << endl;
    }
    else {
        cout << "No hay ruta con " << aero << endl;
    }
}

int main()
{
    unordered_map<int, Aeropuerto> aeropuertos;
    unordered_map<string, int> codigosIATA;
    unordered_map<string, int> nombresAeropuertos;
    leerAeropuertos(aeropuertos, codigosIATA, nombresAeropuertos, "airports_clean.csv");
    leerRutas(aeropuertos, "routes.dat");
    mostrarAeropuertoPorId(aeropuertos, 50);
    reto1(nombresAeropuertos, codigosIATA, aeropuertos);
    reto2(aeropuertos);
    reto3(aeropuertos);
    reto4(aeropuertos);
    reto5(aeropuertos, codigosIATA);
    return 0;
}