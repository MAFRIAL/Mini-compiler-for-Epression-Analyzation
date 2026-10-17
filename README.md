# LexiLab — Mini Compiler / Expression Evaluator

A professional browser GUI backed by a real C++17 expression evaluator. The C++ backend performs lexical analysis, parsing and evaluation.

## Features
- Token analysis with token type, lexeme and source position
- Addition, subtraction, multiplication and division
- Decimal numbers, unary plus/minus and parentheses
- Standard operator precedence
- Errors for invalid characters, malformed expressions and division by zero
- Responsive dark developer-tool interface
- Docker deployment compatible with Render

## Project structure
```text
mini_compiler_gui_project/
├── CMakeLists.txt
├── Dockerfile
├── main.cpp
├── README.md
└── public/
    ├── index.html
    ├── styles.css
    └── app.js
```

## Deploy on Render
1. Create a **new GitHub repository** for this project, separate from any other app you want to keep online.
2. Upload all files from this folder and preserve the `public` folder structure.
3. In Render, select **New → Web Service**, connect the new repository and select **Docker**.
4. Choose the Free instance if available and deploy. The server reads Render's `PORT` variable and listens on `0.0.0.0`.

## Run locally
Requirements: CMake, C++17 compiler, Git and internet access for first-time dependency download.
```bash
cmake -S . -B build
cmake --build build -j2
```
Run the executable from the project root so it can find `public`. Visit `http://localhost:8080` by default. Set `PORT=8080` if needed.

## API
- `GET /api/health` — service health check
- `POST /api/evaluate` — JSON body such as `{"expression":"10 + 5 * 2"}`

Successful responses contain `tokens`, `result` and `formattedResult`. Errors contain `ok: false` and an `error` message.

## Limitations
This is an educational expression evaluator, not a full C++ compiler. It supports numeric arithmetic only, not variables, functions, exponentiation or implicit multiplication.
