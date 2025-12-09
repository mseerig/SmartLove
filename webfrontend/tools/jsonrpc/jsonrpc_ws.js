/**
 * Klasse zur Kommunikation mit einem JSON-RPC-Server über WebSockets.
 * @class jsonRPC_ws
 * @example
 * const wsUrl = "ws://localhost:8080";
 * const rpc = new jsonRPC_ws(wsUrl);
 * rpc.call("system.getHeapSize", "abc").then(result => {
 *    console.log("Result:", result);
 * }).catch(error => {
 *   console.error("Error:", error);
 * });
 * @version 1.0
 * @since 2025-03-01
 */
class jsonRPC_ws {

    /**
     * Erzeugt eine Instanz der Klasse jsonRPC_ws.
     * @param {string} wsUrl - URL des WebSocket-Servers
     * @memberof jsonRPC_ws
     */
    constructor(wsUrl) {
        this.idCounter = 1; // Zähler für eindeutige JSON-RPC-Anfragen
        this.pendingRequests = {}; // Objekt zur Verwaltung ausstehender Anfragen
        this.wsUrl = wsUrl; // URL des WebSocket-Servers
        this.connect(); // Verbindung zum WebSocket-Server herstellen
        this.previousReadyState = null; // Vorheriger Verbindungsstatus

        
        // Überwachung des Verbindungsstatus starten
        this.checkInterval = setInterval(() => {

            // Überprüfen, ob sich der Verbindungsstatus geändert hat
            if (this.ws.readyState !== this.previousReadyState) {
                this.previousReadyState = this.ws.readyState;
                console.log("WebSocket readyState:", this.ws.readyState);

                if (this.ws.readyState === WebSocket.OPEN) {
                    this.onConnected(); // Verbindung geöffnet
                } else {
                    this.onDisconnected(); // Verbindung geschlossen
                }
            }
        }, 100); // Überprüft den Verbindungsstatus alle 100 Millisekunden
    }

    /**
     * Stellt eine Verbindung zum WebSocket-Server her.
     * Legt Event-Handler für onmessage, onerror und onclose fest.
     * @returns {void}
     * @memberof jsonRPC_ws
     * @private
     */
    connect() {
        this.ws = new WebSocket(this.wsUrl); // WebSocket-Verbindung herstellen

        // Event-Handler für WebSocket-Data
        this.ws.onmessage = (event) => {
            this.onConnected();
            
            // Eingehende Nachrichten verarbeiten
            try {
                const response = JSON.parse(event.data); // JSON-Daten parsen
                const { id, result, error } = response; // JSON-Daten extrahieren

                // Überprüfen, ob die Anfrage-ID in pendingRequests vorhanden ist
                if (this.pendingRequests[id]) {
                    clearTimeout(this.pendingRequests[id].timeout); // Timeout löschen
                    if (error) {
                        this.pendingRequests[id].reject(console.error("Websocket Error: "+error)); // Promise ablehnen
                    } else {
                        this.pendingRequests[id].resolve(result); // Promise auflösen
                    }
                    delete this.pendingRequests[id]; // Anfrage aus pendingRequests entfernen
                } else {
                    // Wenn keine ausstehende Anfrage gefunden wird, handleAsync aufrufen
                    this.handleAsync(response); // JSON-Daten verarbeiten
                }

            } catch (e) {
                // Wenn JSON-Parsing fehlschlägt, handleAsync aufrufen
                this.handleAsync(event.data); // Rohdaten verarbeiten
            }
        };

        // Event-Handler für WebSocket-Errors
        this.ws.onerror = (error) => {
            console.error("WebSocket error:", error); // Fehler protokollieren
            this.onDisconnected(); // Verbindung geschlossen
        };

        // Event-Handler für WebSocket-Close
        this.ws.onclose = () => {
            this.onDisconnected(); // Verbindung geschlossen
            console.warn("WebSocket connection closed, attempting to reconnect...");
            setTimeout(() => {
                this.connect(); // Verbindung nach 5 Sekunden wiederherstellen
            }, 1000); // 1 Sekunden warten
        };
    }

    /**
     * Callback-Funktion, die aufgerufen wird, wenn die Verbindung zum WebSocket-Server hergestellt wird.
     */
    onConnected() {
        console.log("Connected to WebSocket"); // Verbindung geöffnet
    }

    /**
     * Callback-Funktion, die aufgerufen wird, wenn die Verbindung zum WebSocket-Server geschlossen wird.
     */
    onDisconnected() {
        console.log("Disconnected from WebSocket"); // Verbindung geschlossen
    }

    /**
     * Callback-Funktion, die aufgerufen wird, wenn asynchrone Daten empfangen werden.
     * @param {Object} data - Empfangene Daten
     */
    handleAsync(data) {
        // Asynchrone Daten verarbeiten
        if (this.onAsyncData) {
            this.onAsyncData(data); // Callback aufrufen, wenn gesetzt
        } else {
            // Wenn kein Callback gesetzt ist, Daten protokollieren
            console.log("Received data:", data);
        }
    }

    /**
     * Sendet eine JSON-RPC-Anfrage an den Server.
     * @param {string} method - Name der Methode
     * @param {Array} params - Parameter der Methode
     * @returns {Promise} - Promise-Objekt
     * @memberof jsonRPC_ws
     */
    call(method, params) {
        return new Promise((resolve, reject) => {
            const id = this.idCounter++; // Eindeutige Anfrage-ID generieren
            const request = {
                jsonrpc: "2.0",
                method,
                params,
                id
            };

            // Timeout für Anfrage setzen
            const timeoutId = setTimeout(() => {
                if (this.pendingRequests[id]) { // Überprüfen, ob Anfrage noch ausstehend ist
                    reject(console.warn("Websocket Timeout")); // Anfrage ablehnen bei Timeout
                    delete this.pendingRequests[id]; // Anfrage aus pendingRequests entfernen
                    this.onDisconnected(); // Verbindung geschlossen
                }
            }, 500); // Timeout auf 5 Sekunden setzen

            this.pendingRequests[id] = { resolve, reject, timeout: timeoutId }; // Anfrage speichern

            if (this.ws.readyState === WebSocket.OPEN) {
                this.ws.send(JSON.stringify(request)); // Anfrage senden, wenn Verbindung geöffnet ist
            } else {
                clearTimeout(timeoutId); // Timeout löschen
                reject(console.warn("Websocket Closed")); // Anfrage ablehnen, wenn Verbindung nicht geöffnet ist
                this.onDisconnected(); // Verbindung geschlossen
            }
        });
    }
}