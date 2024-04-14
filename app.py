from flask import Flask, render_template, request, jsonify, redirect, url_for, session
from flask_mysqldb import MySQL, MySQLdb
from os import path
from notifypy import Notify

app = Flask(__name__)
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = ''
app.config['MYSQL_DB'] = 'app_citas'
app.config['MYSQL_CURSORCLASS'] = 'DictCursor'
mysql = MySQL(app)

datos_recibidos = []

@app.route('/')
def home():
    return render_template("contenido.html")

@app.route('/layout', methods=["GET", "POST"])
def layout():
    session.clear()
    return render_template("contenido.html")

@app.route('/pe')
def pe():
    return render_template('pe.html')

@app.route('/cedula_home', methods=['GET', 'POST'])
def cedula_home():
    if request.method == 'POST':
        data = request.get_json()
        breathing_rate = data.get('breathingRate')
        spo2 = data.get('spo2')

        # Haz lo que desees con breathing_rate y spo2
        print("Valor de breathingRate recibido:", breathing_rate)
        print("Valor de spo2 recibido:", spo2)

        datos_recibidos.append({'breathingRate': breathing_rate, 'spo2': spo2})

        # Dvolver una respuesta JSON
        return jsonify({'message': 'Datos recibidos correctamente'})

    elif 'email' in session and session['tipo'] == 1:
        return render_template("Cedula/home.html")
    elif 'email' in session and session['tipo'] == 2:
        return render_template("Tarjeta/homeTwo.html")
    else:
        return redirect(url_for('login'))

@app.route('/grafica', methods=['GET'])
def obtener_datos_grafica():
    # Obtener los últimos datos enviados desde el ESP32
    if datos_recibidos:
        ultimo_dato = datos_recibidos[-1]
        return jsonify(ultimo_dato)
    else:
        return jsonify({})

@app.route ('/login', methods=["GET", "POST"])
def login():
    notificacion = Notify()

    if request.method == 'POST':
        email = request.form['email']
        password = request.form['password']

        cur = mysql.connection.cursor()
        cur.execute("SELECT * FROM users WHERE email=%s", (email,))
        user = cur.fetchone()
        cur.close()

        if user:
            if password == user["password"]:
                session['name'] = user['name']
                session['email'] = user['email']
                session['tipo'] = user['id_tip_usu']

                if session['tipo'] == 1:
                    return render_template("Cedula/home.html")
                elif session['tipo'] == 2:
                    return render_template("Tarjeta/homeTwo.html")
            else:
                notificacion.title = "Error de Acceso"
                notificacion.message = "Correo o contraseña no valida"
                notificacion.send()
                return render_template("login.html")
        else:
            notificacion.title = "Error de Acceso"
            notificacion.message = "No existe el usuario"
            notificacion.send()
            return render_template("login.html")
    else:
        return render_template("login.html")

@app.route('/registro', methods=["GET", "POST"])
def registro():
    cur = mysql.connection.cursor()
    cur.execute("SELECT * FROM tip_usu")
    tipo = cur.fetchall()

    cur.close()

    notificacion = Notify()

    if request.method == 'GET':
        return render_template("registro.html", tipo=tipo)

    else:
        name = request.form['name']
        email = request.form['email']
        password = request.form['password']
        tip = request.form['tipo']
        N_id = request.form['N_id']

        cur = mysql.connection.cursor()
        cur.execute("INSERT INTO users (name, email, password, id_tip_usu, N_id) VALUES (%s, %s, %s, %s, %s)",
                    (name, email, password, tip, N_id))
        mysql.connection.commit()
        notificacion.title = "Registro Exitoso"
        notificacion.message = "ya te encuentras registrado en RespiraCHECK, por favor inicia sesión."
        notificacion.send()
        return redirect(url_for('login'))
    
if __name__ == '__main__':
    app.secret_key = "pinchellave"
    app.run(debug=True)

