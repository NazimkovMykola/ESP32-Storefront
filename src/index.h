#ifndef INDEX_H
#define INDEX_H

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Matrix Control</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            margin-top: 50px; 
            background: #f0f0f0; 
            color: #333;
        }
        form { 
            display: inline-block; 
            background: white; 
            padding: 25px; 
            border-radius: 12px; 
            box-shadow: 0 4px 15px rgba(0,0,0,0.1); 
            width: 90%;
            max-width: 400px;
        }
        h2 { color: #2c3e50; margin-bottom: 20px; }
        label { font-weight: bold; display: block; margin-top: 15px; text-align: left; }
        input[type="text"] { 
            width: 100%; 
            padding: 12px; 
            margin: 8px 0; 
            border: 1px solid #ccc; 
            border-radius: 6px; 
            box-sizing: border-box;
            font-size: 16px;
        }
        input[type="submit"] { 
            background: #4CAF50; 
            color: white; 
            padding: 12px 20px; 
            border: none; 
            border-radius: 6px; 
            cursor: pointer; 
            width: 100%;
            font-size: 16px;
            margin-top: 20px;
            font-weight: bold;
        }
        input[type="submit"]:hover { background: #45a049; }
    </style>
</head>
<body>
    <form action="/get">
        <h2>Керування LED Панеллю</h2>
        
        <label>Верхній рядок (статичний):</label>
        <input type="text" name="top_text" value="%TOP_VAL%" maxlength="60">
        
        <label>Нижній рядок (бігучий):</label>
        <input type="text" name="btm_text" value="%BTM_VAL%" maxlength="250">
        
        <input type="submit" value="Оновити екран">
    </form>
</body>
</html>
)rawliteral";

#endif