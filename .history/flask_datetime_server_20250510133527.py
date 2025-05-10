# Importamos Flask para crear el servidor web
from flask import Flask
# Importamos datetime para obtener la fecha y hora actual
from datetime import datetime

# Creamos una instancia de la aplicación Flask
app = Flask(__name__)

# Definimos una ruta que responde a solicitudes GET en "/datetime"
@app.route("/datetime")
def get_datetime():
    # Obtenemos la fecha y hora actual
    now = datetime.now()
    # La devolvemos en formato cadena legible
    return now.strftime("%Y-%m-%d %H:%M:%S")

# Si este archivo se ejecuta directamente, iniciamos el servidor
if __name__ == "__main__":
    # Ejecutamos la aplicación en todas las interfaces de red (0.0.0.0) en el puerto 5000
    app.run(host="0.0.0.0", port=5000)