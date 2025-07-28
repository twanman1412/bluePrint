const fs = require('fs');
const path = require('path');
const { marked } = require('marked');
const hljs = require('highlight.js');

// Configure marked to use highlight.js
marked.setOptions({
  highlight: function(code, lang) {
    // Use Java syntax highlighting for blueprint code with custom enhancements
    if (lang === 'blueprint') {
      lang = 'java';
      
      // After highlighting with Java, add custom BluePrint type highlighting
      const language = hljs.getLanguage(lang) ? lang : 'plaintext';
      let highlighted = hljs.highlight(code, { language }).value;
      
      // Define BluePrint-specific types
      const blueprintTypes = [
        'i8', 'i16', 'i32', 'i64', 'i128',
        'u8', 'u16', 'u32', 'u64', 'u128',
        'f32', 'f64',
        'bool', 'str', 'char',
        'void', 'var', 'final',
        'blueprint'
      ];
      
      // Create a regex pattern for BluePrint types
      const typePattern = new RegExp(`\\b(${blueprintTypes.join('|')})\\b`, 'g');
      
      // Replace BluePrint types with proper highlighting
      highlighted = highlighted.replace(typePattern, '<span class="hljs-type">$1</span>');
      
      // Also highlight 'blueprint' keyword specifically
      highlighted = highlighted.replace(/\bblueprint\b/g, '<span class="hljs-keyword">blueprint</span>');
      
      return highlighted;
    }
    
    const language = hljs.getLanguage(lang) ? lang : 'plaintext';
    return hljs.highlight(code, { language }).value;
  },
  langPrefix: 'hljs language-',
  breaks: false,
  gfm: true
});

// Create build directory if it doesn't exist
const buildDir = path.join(__dirname, 'build');
if (!fs.existsSync(buildDir)) {
  fs.mkdirSync(buildDir, { recursive: true });
}

// Create img directory in build and copy assets
const imgBuildDir = path.join(buildDir, 'img');
const imgSourceDir = path.join(__dirname, 'img');

// Function to copy directory recursively
function copyDir(src, dest) {
  if (!fs.existsSync(dest)) {
    fs.mkdirSync(dest, { recursive: true });
  }
  
  const items = fs.readdirSync(src);
  for (const item of items) {
    const srcPath = path.join(src, item);
    const destPath = path.join(dest, item);
    
    if (fs.statSync(srcPath).isDirectory()) {
      copyDir(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}

// Copy all images to build directory
if (fs.existsSync(imgSourceDir)) {
  copyDir(imgSourceDir, imgBuildDir);
  console.log('Assets copied to build directory');
}

// Define documentation structure
const docStructure = [
  { file: 'index.md', title: 'Introduction' },
  { file: 'syntax.md', title: 'Syntax' },
  { file: 'blueprints.md', title: 'Blueprint Specifications' },
  { file: 'types.md', title: 'Type System' },
  { file: 'functions.md', title: 'Functions' },
  { file: 'control-flow.md', title: 'Control Flow' },
  { file: 'concurrency.md', title: 'Concurrency' },
  { file: 'modules.md', title: 'Modules' },
  { file: 'stdlib.md', title: 'Standard Library' },
  { file: 'examples.md', title: 'Examples' }
];

// Read and combine all markdown files
let combinedContent = '';
let tableOfContents = '## Table of Contents\n\n';

docStructure.forEach((doc, index) => {
  const filePath = path.join(__dirname, doc.file);
  
  if (fs.existsSync(filePath)) {
    const content = fs.readFileSync(filePath, 'utf8');
    combinedContent += `\n\n<!-- Section ${index + 1}: ${doc.title} -->\n`;
    combinedContent += content;
    
    // Add to table of contents
    const anchor = doc.title.toLowerCase().replace(/\s+/g, '-').replace(/[^\w-]/g, '');
    tableOfContents += `${index + 1}. [${doc.title}](#${anchor})\n`;
  } else {
    console.warn(`Warning: ${doc.file} not found, skipping...`);
  }
});

// Add table of contents at the beginning
const fullContent = tableOfContents + '\n' + combinedContent;

// Convert markdown to HTML
const htmlContent = marked(fullContent);

// Create the complete HTML page
const htmlPage = `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>BluePrint Programming Language Documentation</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.8.0/styles/github.min.css">
    <link rel="icon" type="image/png" sizes="32x32" href="img/bluePrint_logo/BluePrint_logo_32px.png">
    <link rel="icon" type="image/png" sizes="16x16" href="img/bluePrint_logo/BluePrint_logo_16px.png">
    <style>
        :root {
            --primary-blue: #2563eb;
            --secondary-blue: #1e40af;
            --light-blue: #dbeafe;
            --dark-blue: #1e3a8a;
            --gray-50: #f9fafb;
            --gray-100: #f3f4f6;
            --gray-200: #e5e7eb;
            --gray-300: #d1d5db;
            --gray-600: #4b5563;
            --gray-700: #374151;
            --gray-800: #1f2937;
            --gray-900: #111827;
            --success-green: #10b981;
            --warning-orange: #f59e0b;
            
            /* Light theme colors */
            --bg-primary: #ffffff;
            --bg-secondary: #f8fafc;
            --bg-gradient-start: #f1f5f9;
            --bg-gradient-end: #e2e8f0;
            --text-primary: #1f2937;
            --text-secondary: #374151;
            --text-muted: #6b7280;
            --border-color: #e5e7eb;
            --code-bg: #f1f5f9;
            --code-border: #cbd5e1;
            --pre-bg: #334155;
            --pre-text: #f1f5f9;
            --toc-bg-start: #dbeafe;
            --toc-bg-end: #f0f9ff;
            --blockquote-bg: #fffbeb;
            --table-hover: #f9fafb;
        }

        @media (prefers-color-scheme: dark) {
            :root {
                /* Dark theme colors */
                --bg-primary: #1e293b;
                --bg-secondary: #0f172a;
                --bg-gradient-start: #020617;
                --bg-gradient-end: #0f172a;
                --text-primary: #f1f5f9;
                --text-secondary: #cbd5e1;
                --text-muted: #94a3b8;
                --border-color: #334155;
                --code-bg: #0f172a;
                --code-border: #475569;
                --pre-bg: #020617;
                --pre-text: #e2e8f0;
                --toc-bg-start: #1e3a8a;
                --toc-bg-end: #1e40af;
                --blockquote-bg: #451a03;
                --table-hover: #334155;
                
                /* Adjust other colors for dark theme */
                --primary-blue: #3b82f6;
                --secondary-blue: #2563eb;
                --light-blue: #1e3a8a;
                --dark-blue: #3b82f6;
                --success-green: #34d399;
                --warning-orange: #fbbf24;
                --gray-50: #1e293b;
                --gray-100: #334155;
                --gray-200: #475569;
                --gray-300: #64748b;
                --gray-600: #94a3b8;
                --gray-700: #cbd5e1;
                --gray-800: #e2e8f0;
                --gray-900: #f1f5f9;
            }
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', sans-serif;
            line-height: 1.7;
            color: var(--text-primary);
            background: linear-gradient(135deg, var(--bg-gradient-start) 0%, var(--bg-gradient-end) 100%);
            min-height: 100vh;
            transition: background-color 0.3s ease, color 0.3s ease;
        }

        .header {
            background: linear-gradient(135deg, var(--primary-blue) 0%, var(--secondary-blue) 100%);
            color: white;
            padding: 2rem 0;
            text-align: center;
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            position: relative;
            overflow: hidden;
        }

        .header::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: url('img/bluePrint_banner/BluePrint_banner_top.png') center/cover;
            opacity: 0.1;
            z-index: 0;
        }

        .header-content {
            position: relative;
            z-index: 1;
            max-width: 1200px;
            margin: 0 auto;
            padding: 0 2rem;
        }

        .logo {
            width: 80px;
            height: 80px;
            margin: 0 auto 1rem;
            display: block;
            filter: drop-shadow(0 4px 8px rgba(0, 0, 0, 0.2));
        }

        .header h1 {
            font-size: 3rem;
            font-weight: 800;
            margin-bottom: 0.5rem;
            text-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
            border: none;
            color: white;
        }

        .header .subtitle {
            font-size: 1.25rem;
            opacity: 0.9;
            font-weight: 300;
        }

        .theme-toggle {
            position: absolute;
            top: 1rem;
            right: 2rem;
            background: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.2);
            color: white;
            border-radius: 8px;
            padding: 0.5rem 1rem;
            cursor: pointer;
            font-size: 0.875rem;
            transition: all 0.2s ease;
            backdrop-filter: blur(10px);
        }

        .theme-toggle:hover {
            background: rgba(255, 255, 255, 0.2);
            transform: translateY(-1px);
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
            background: var(--bg-primary);
            min-height: calc(100vh - 200px);
            box-shadow: 0 0 20px rgba(0, 0, 0, 0.1);
            border-radius: 12px;
            margin-top: -2rem;
            position: relative;
            z-index: 2;
            transition: background-color 0.3s ease;
        }

        .toc {
            background: linear-gradient(135deg, var(--toc-bg-start) 0%, var(--toc-bg-end) 100%);
            padding: 2rem;
            border-radius: 12px;
            margin: 2rem 0;
            border-left: 4px solid var(--primary-blue);
            box-shadow: 0 2px 10px rgba(37, 99, 235, 0.1);
            transition: background 0.3s ease;
        }

        .toc h2 {
            color: var(--dark-blue);
            margin-bottom: 1rem;
            font-size: 1.5rem;
            display: flex;
            align-items: center;
        }

        .toc h2::before {
            content: '📋';
            margin-right: 0.5rem;
        }

        .toc ol {
            counter-reset: section;
            list-style: none;
            padding-left: 0;
        }

        .toc li {
            counter-increment: section;
            margin: 0.5rem 0;
            padding-left: 2rem;
            position: relative;
        }

        .toc li::before {
            content: counter(section) ".";
            position: absolute;
            left: 0;
            color: var(--primary-blue);
            font-weight: 600;
        }

        .toc a {
            color: var(--text-secondary);
            text-decoration: none;
            padding: 0.25rem 0.5rem;
            border-radius: 6px;
            transition: all 0.2s ease;
            display: inline-block;
        }

        .toc a:hover {
            background: var(--bg-primary);
            color: var(--primary-blue);
            transform: translateX(4px);
        }

        h1, h2, h3, h4, h5, h6 {
            color: var(--text-primary);
            margin-top: 3rem;
            margin-bottom: 1rem;
            font-weight: 700;
            line-height: 1.3;
        }

        h1 {
            font-size: 2.5rem;
            border-bottom: 3px solid var(--primary-blue);
            padding-bottom: 1rem;
            position: relative;
        }

        h1::after {
            content: '';
            position: absolute;
            bottom: -3px;
            left: 0;
            width: 60px;
            height: 3px;
            background: var(--success-green);
        }

        h2 {
            font-size: 2rem;
            border-bottom: 2px solid var(--border-color);
            padding-bottom: 0.5rem;
            color: var(--dark-blue);
        }

        h3 {
            font-size: 1.5rem;
            color: var(--primary-blue);
        }

        h4 {
            font-size: 1.25rem;
            color: var(--text-secondary);
        }

        p {
            margin-bottom: 1.5rem;
            color: var(--text-secondary);
        }

        code {
            background: var(--code-bg);
            padding: 0.25rem 0.5rem;
            border-radius: 4px;
            font-family: 'JetBrains Mono', 'Fira Code', 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
            font-size: 0.9em;
            color: var(--primary-blue);
            border: 1px solid var(--code-border);
            transition: background-color 0.3s ease, border-color 0.3s ease;
        }

        pre {
            background: var(--pre-bg);
            color: var(--pre-text);
            padding: 1.5rem;
            border-radius: 8px;
            overflow-x: auto;
            margin: 1.5rem 0;
            border-left: 4px solid var(--primary-blue);
            box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
            position: relative;
            transition: background-color 0.3s ease;
        }

        pre::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: linear-gradient(90deg, var(--primary-blue), var(--success-green));
        }

        pre code {
            background: transparent;
            padding: 0;
            border: none;
            color: inherit;
            font-size: 0.875rem;
        }

        blockquote {
            border-left: 4px solid var(--warning-orange);
            padding: 1rem 1.5rem;
            margin: 1.5rem 0;
            background: var(--blockquote-bg);
            border-radius: 0 8px 8px 0;
            color: var(--text-secondary);
            position: relative;
            transition: background-color 0.3s ease;
        }

        blockquote::before {
            content: '💡';
            position: absolute;
            top: 1rem;
            left: -2px;
            background: var(--warning-orange);
            color: white;
            width: 24px;
            height: 24px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 12px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin: 2rem 0;
            background: var(--bg-primary);
            border-radius: 8px;
            overflow: hidden;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            transition: background-color 0.3s ease;
        }

        th, td {
            padding: 1rem;
            text-align: left;
            border-bottom: 1px solid var(--border-color);
        }

        th {
            background: linear-gradient(135deg, var(--primary-blue), var(--secondary-blue));
            color: white;
            font-weight: 600;
            text-transform: uppercase;
            font-size: 0.875rem;
            letter-spacing: 0.025em;
        }

        tr:hover {
            background: var(--table-hover);
        }

        ul, ol {
            margin: 1rem 0 1.5rem 2rem;
            color: var(--text-secondary);
        }

        li {
            margin: 0.5rem 0;
        }

        a {
            color: var(--primary-blue);
            text-decoration: none;
            transition: color 0.2s ease;
        }

        a:hover {
            color: var(--secondary-blue);
            text-decoration: underline;
        }

        .footer {
            text-align: center;
            padding: 2rem;
            color: var(--text-muted);
            border-top: 1px solid var(--border-color);
            margin-top: 3rem;
        }

        /* Enhanced syntax highlighting for better visibility */
        .hljs {
            background: var(--pre-bg) !important;
            color: var(--pre-text) !important;
        }

        /* Light theme syntax highlighting */
        @media (prefers-color-scheme: light) {
            .hljs-keyword,
            .hljs-built_in {
                color: #d73a49 !important;
            }

            .hljs-string {
                color: #032f62 !important;
            }

            .hljs-comment {
                color: #6a737d !important;
                font-style: italic;
            }

            .hljs-number {
                color: #005cc5 !important;
            }

            .hljs-type {
                color: #6f42c1 !important;
                font-weight: 600;
            }

            .hljs-variable {
                color: #e36209 !important;
            }

            .hljs-function {
                color: #6f42c1 !important;
            }

            .hljs-title {
                color: #6f42c1 !important;
            }
        }

        /* Dark theme syntax highlighting */
        @media (prefers-color-scheme: dark) {
            .hljs-keyword,
            .hljs-built_in {
                color: #ff7b72 !important;
            }

            .hljs-string {
                color: #a5d6ff !important;
            }

            .hljs-comment {
                color: #8b949e !important;
                font-style: italic;
            }

            .hljs-number {
                color: #79c0ff !important;
            }

            .hljs-type {
                color: #ffa657 !important;
                font-weight: 600;
            }

            .hljs-variable {
                color: #ffa657 !important;
            }

            .hljs-function {
                color: #d2a8ff !important;
            }

            .hljs-title {
                color: #d2a8ff !important;
            }
        }

        /* Manual light theme syntax highlighting */
        .theme-manual-light .hljs-keyword,
        .theme-manual-light .hljs-built_in {
            color: #d73a49 !important;
        }

        .theme-manual-light .hljs-string {
            color: #032f62 !important;
        }

        .theme-manual-light .hljs-comment {
            color: #6a737d !important;
            font-style: italic;
        }

        .theme-manual-light .hljs-number {
            color: #005cc5 !important;
        }

        .theme-manual-light .hljs-type {
            color: #6f42c1 !important;
            font-weight: 600;
        }

        .theme-manual-light .hljs-variable {
            color: #e36209 !important;
        }

        .theme-manual-light .hljs-function {
            color: #6f42c1 !important;
        }

        .theme-manual-light .hljs-title {
            color: #6f42c1 !important;
        }

        /* Manual dark theme syntax highlighting */
        .theme-manual-dark .hljs-keyword,
        .theme-manual-dark .hljs-built_in {
            color: #ff7b72 !important;
        }

        .theme-manual-dark .hljs-string {
            color: #a5d6ff !important;
        }

        .theme-manual-dark .hljs-comment {
            color: #8b949e !important;
            font-style: italic;
        }

        .theme-manual-dark .hljs-number {
            color: #79c0ff !important;
        }

        .theme-manual-dark .hljs-type {
            color: #ffa657 !important;
            font-weight: 600;
        }

        .theme-manual-dark .hljs-variable {
            color: #ffa657 !important;
        }

        .theme-manual-dark .hljs-function {
            color: #d2a8ff !important;
        }

        .theme-manual-dark .hljs-title {
            color: #d2a8ff !important;
        }

        /* Responsive design */
        @media (max-width: 768px) {
            .container {
                padding: 1rem;
                margin-top: -1rem;
            }
            
            .header h1 {
                font-size: 2rem;
            }
            
            .toc {
                padding: 1rem;
            }
            
            pre {
                padding: 1rem;
                font-size: 0.8rem;
            }

            .theme-toggle {
                position: static;
                margin: 1rem auto 0;
                display: block;
                width: fit-content;
            }
        }

        /* Smooth scrolling */
        html {
            scroll-behavior: smooth;
        }

        /* Loading animation */
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .container > * {
            animation: fadeIn 0.6s ease-out;
        }

        /* Manual theme toggle styles */
        .theme-manual-dark {
            --bg-primary: #1e293b;
            --bg-secondary: #0f172a;
            --bg-gradient-start: #020617;
            --bg-gradient-end: #0f172a;
            --text-primary: #f1f5f9;
            --text-secondary: #cbd5e1;
            --text-muted: #94a3b8;
            --border-color: #334155;
            --code-bg: #0f172a;
            --code-border: #475569;
            --pre-bg: #020617;
            --pre-text: #e2e8f0;
            --toc-bg-start: #1e3a8a;
            --toc-bg-end: #1e40af;
            --blockquote-bg: #451a03;
            --table-hover: #334155;
            --primary-blue: #3b82f6;
            --secondary-blue: #2563eb;
            --light-blue: #1e3a8a;
            --dark-blue: #3b82f6;
            --success-green: #34d399;
            --warning-orange: #fbbf24;
        }

        .theme-manual-light {
            --bg-primary: #ffffff;
            --bg-secondary: #f8fafc;
            --bg-gradient-start: #f1f5f9;
            --bg-gradient-end: #e2e8f0;
            --text-primary: #1f2937;
            --text-secondary: #374151;
            --text-muted: #6b7280;
            --border-color: #e5e7eb;
            --code-bg: #f1f5f9;
            --code-border: #cbd5e1;
            --pre-bg: #334155;
            --pre-text: #f1f5f9;
            --toc-bg-start: #dbeafe;
            --toc-bg-end: #f0f9ff;
            --blockquote-bg: #fffbeb;
            --table-hover: #f9fafb;
            --primary-blue: #2563eb;
            --secondary-blue: #1e40af;
            --light-blue: #dbeafe;
            --dark-blue: #1e3a8a;
            --success-green: #10b981;
            --warning-orange: #f59e0b;
        }
    </style>
</head>
<body>
    <header class="header">
        <div class="header-content">
            <button class="theme-toggle" onclick="toggleTheme()">🌓 Toggle Theme</button>
            <img src="img/bluePrint_logo/BluePrint_logo_128px.png" alt="BluePrint Logo" class="logo">
            <h1>BluePrint</h1>
            <p class="subtitle">Programming Language Documentation</p>
        </div>
    </header>
    
    <div class="container">
        ${htmlContent}
        
        <footer class="footer">
            <p>BluePrint Programming Language Documentation</p>
            <p>Built with ❤️ using the BluePrint documentation build system</p>
        </footer>
    </div>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.8.0/highlight.min.js"></script>
    <script>
        hljs.highlightAll();
        
        // Theme management
        function toggleTheme() {
            const body = document.body;
            const currentTheme = localStorage.getItem('theme');
            
            if (currentTheme === 'dark') {
                body.classList.remove('theme-manual-dark');
                body.classList.add('theme-manual-light');
                localStorage.setItem('theme', 'light');
            } else if (currentTheme === 'light') {
                body.classList.remove('theme-manual-light');
                body.classList.remove('theme-manual-dark');
                localStorage.removeItem('theme');
            } else {
                // Auto theme - toggle to dark
                body.classList.add('theme-manual-dark');
                localStorage.setItem('theme', 'dark');
            }
            
            updateThemeButton();
        }
        
        function updateThemeButton() {
            const button = document.querySelector('.theme-toggle');
            const currentTheme = localStorage.getItem('theme');
            
            if (currentTheme === 'dark') {
                button.textContent = '🌙 Dark Mode';
            } else if (currentTheme === 'light') {
                button.textContent = '☀️ Light Mode';
            } else {
                button.textContent = '🌓 Auto Theme';
            }
        }
        
        // Initialize theme on page load
        function initTheme() {
            const savedTheme = localStorage.getItem('theme');
            const body = document.body;
            
            if (savedTheme === 'dark') {
                body.classList.add('theme-manual-dark');
            } else if (savedTheme === 'light') {
                body.classList.add('theme-manual-light');
            }
            
            updateThemeButton();
        }
        
        // Initialize theme when DOM is loaded
        document.addEventListener('DOMContentLoaded', initTheme);
    </script>
</body>
</html>`;

// Write the HTML file
fs.writeFileSync(path.join(buildDir, 'index.html'), htmlPage);

console.log('Documentation built successfully!');
console.log('Open build/index.html in your browser to view the documentation.');
console.log('Or run "npm run serve" to start a local server at http://localhost:3000');
