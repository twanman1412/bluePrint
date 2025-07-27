const fs = require('fs');
const path = require('path');
const { marked } = require('marked');
const hljs = require('highlight.js');

// Configure marked to use highlight.js
marked.setOptions({
  highlight: function(code, lang) {
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
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            line-height: 1.6;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            color: #333;
        }
        h1, h2, h3, h4, h5, h6 {
            color: #2c3e50;
            margin-top: 2em;
        }
        h1 {
            border-bottom: 3px solid #3498db;
            padding-bottom: 10px;
        }
        h2 {
            border-bottom: 1px solid #ecf0f1;
            padding-bottom: 5px;
        }
        code {
            background-color: #f8f9fa;
            padding: 2px 4px;
            border-radius: 3px;
            font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
        }
        pre {
            background-color: #f8f9fa;
            padding: 16px;
            border-radius: 6px;
            overflow-x: auto;
            border-left: 4px solid #3498db;
        }
        pre code {
            background-color: transparent;
            padding: 0;
        }
        blockquote {
            border-left: 4px solid #3498db;
            padding-left: 16px;
            margin-left: 0;
            color: #7f8c8d;
        }
        table {
            border-collapse: collapse;
            width: 100%;
            margin: 1em 0;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 8px 12px;
            text-align: left;
        }
        th {
            background-color: #f8f9fa;
            font-weight: 600;
        }
        .toc {
            background-color: #f8f9fa;
            padding: 20px;
            border-radius: 6px;
            margin: 20px 0;
        }
        .toc ul {
            margin: 0;
            padding-left: 20px;
        }
    </style>
</head>
<body>
    <div class="toc">
        ${htmlContent}
    </div>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.8.0/highlight.min.js"></script>
    <script>hljs.highlightAll();</script>
</body>
</html>`;

// Write the HTML file
fs.writeFileSync(path.join(buildDir, 'index.html'), htmlPage);

console.log('Documentation built successfully!');
console.log('Open build/index.html in your browser to view the documentation.');
console.log('Or run "npm run serve" to start a local server at http://localhost:3000');
