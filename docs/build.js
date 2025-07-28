const fs = require('fs');
const path = require('path');
const { marked } = require('marked');
const hljs = require('highlight.js');

// Configure marked to use highlight.js
marked.setOptions({
  highlight: function(code, lang) {
    // Use Java syntax highlighting for blueprint code with custom enhancements
    // Supports both .bp (implementation) and .bpf (blueprint-only) files
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
  { file: 'types.md', title: 'Type System' },
  { file: 'control-flow.md', title: 'Control Flow' },
  { file: 'functions.md', title: 'Functions' },
  { file: 'blueprints.md', title: 'Blueprint Specifications' },
  { file: 'memory.md', title: 'Memory Management' },
  { file: 'concurrency.md', title: 'Concurrency' },
  { file: 'exceptions.md', title: 'Exception Handling' },
  { file: 'modules.md', title: 'Modules' },
  { file: 'stdlib.md', title: 'Standard Library' },
  { file: 'examples.md', title: 'Examples' }
];

// Read all markdown files and prepare content structure
let sections = [];
let sidebarData = [];

docStructure.forEach((doc, index) => {
  const filePath = path.join(__dirname, 'docs', doc.file);
  
  if (fs.existsSync(filePath)) {
    const content = fs.readFileSync(filePath, 'utf8');
    const htmlContent = marked(content);
    const anchor = doc.title.toLowerCase().replace(/\s+/g, '-').replace(/[^\w-]/g, '');
    
    // Extract headings from markdown content
    const headings = extractHeadings(content);
    
    sections.push({
      id: `section-${index}`,
      title: doc.title,
      content: htmlContent,
      anchor: anchor,
      index: index,
      headings: headings
    });
    
    sidebarData.push({
      index: index,
      title: doc.title,
      headings: headings
    });
  } else {
    console.warn(`Warning: ${doc.file} not found, skipping...`);
  }
});

// Function to extract headings from markdown content
function extractHeadings(content) {
  const headings = [];
  const lines = content.split('\n');
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    
    // Match markdown headings (## or ###)
    const match = line.match(/^(#{2,4})\s+(.+)$/);
    if (match) {
      const level = match[1].length; // Number of # characters
      const text = match[2].trim();
      const id = text.toLowerCase()
        .replace(/[^\w\s-]/g, '') // Remove special characters except spaces and hyphens
        .replace(/\s+/g, '-')     // Replace spaces with hyphens
        .replace(/-+/g, '-')      // Replace multiple hyphens with single
        .replace(/^-|-$/g, '');   // Remove leading/trailing hyphens
      
      headings.push({
        level: level,
        text: text,
        id: id,
        line: i + 1
      });
    }
  }
  
  return headings;
}

// Generate sidebar HTML
function generateSidebarHTML() {
  return sidebarData.map(section => {
    const hasSubheadings = section.headings.length > 0;
    
    return `
      <li class="toc-section" data-section="${section.index}">
        <div class="toc-section-header" onclick="toggleSection(${section.index})" data-section="${section.index}">
          <span class="toc-number">${section.index + 1}</span>
          <span class="toc-title">${section.title}</span>
          ${hasSubheadings ? '<span class="toc-toggle">▶</span>' : ''}
        </div>
        ${hasSubheadings ? `
          <ul class="toc-subsections">
            ${section.headings.map(heading => `
              <li class="toc-subitem level-${heading.level}" data-section="${section.index}" data-heading="${heading.id}">
                <a href="#" onclick="showSectionAndHeading(${section.index}, '${heading.id}'); return false;">
                  ${heading.text}
                </a>
              </li>
            `).join('')}
          </ul>
        ` : ''}
      </li>
    `;
  }).join('');
}

const tableOfContentsHtml = generateSidebarHTML();

// Generate JavaScript sections data
const sectionsData = JSON.stringify(sections.map(s => ({
  id: s.id,
  title: s.title,
  content: s.content,
  index: s.index,
  headings: s.headings
})));

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
            --sidebar-bg: #f8fafc;
            --sidebar-border: #e5e7eb;
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
                --sidebar-bg: #0f172a;
                --sidebar-border: #334155;
                
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
            background: var(--bg-secondary);
            min-height: 100vh;
            transition: background-color 0.3s ease, color 0.3s ease;
            display: flex;
            flex-direction: column;
        }

        .header {
            background: linear-gradient(135deg, var(--primary-blue) 0%, var(--secondary-blue) 100%);
            color: white;
            padding: 1rem 0;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            position: relative;
            overflow: hidden;
            z-index: 1000;
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
            display: flex;
            align-items: center;
            justify-content: space-between;
        }

        .logo-section {
            display: flex;
            align-items: center;
            gap: 1rem;
        }

        .logo {
            width: 48px;
            height: 48px;
            filter: drop-shadow(0 2px 4px rgba(0, 0, 0, 0.2));
        }

        .header h1 {
            font-size: 1.75rem;
            font-weight: 800;
            margin: 0;
            text-shadow: 0 1px 2px rgba(0, 0, 0, 0.3);
            border: none;
            color: white;
        }

        .theme-toggle {
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

        .main-container {
            display: flex;
            flex: 1;
            height: calc(100vh - 100px);
            overflow: hidden;
        }

        .sidebar {
            width: 280px;
            background: var(--sidebar-bg);
            border-right: 1px solid var(--sidebar-border);
            padding: 1.5rem;
            overflow-y: auto;
            transition: background-color 0.3s ease, border-color 0.3s ease;
            box-shadow: 2px 0 4px rgba(0, 0, 0, 0.05);
            position: fixed;
            left: 0;
            top: 100px;
            height: calc(100vh - 100px);
            z-index: 100;
        }

        .sidebar h2 {
            color: var(--dark-blue);
            margin-bottom: 1rem;
            font-size: 1.25rem;
            display: flex;
            align-items: center;
            border: none;
            padding: 0;
        }

        .sidebar h2::before {
            content: '📋';
            margin-right: 0.5rem;
        }

        .toc-list {
            list-style: none;
            padding: 0;
            margin: 0;
        }

        .toc-section {
            margin: 0.25rem 0;
            border-radius: 8px;
            transition: all 0.2s ease;
        }

        .toc-section.active {
            background: linear-gradient(135deg, var(--primary-blue), var(--secondary-blue));
            color: white;
            box-shadow: 0 2px 8px rgba(37, 99, 235, 0.3);
        }

        .toc-section.expanded .toc-toggle {
            transform: rotate(90deg);
        }

        .toc-section-header {
            display: flex;
            align-items: center;
            padding: 0.75rem;
            color: var(--text-secondary);
            border-radius: 8px;
            transition: all 0.2s ease;
            cursor: pointer;
            user-select: none;
        }

        .toc-section.active .toc-section-header {
            color: white;
        }

        .toc-section:not(.active):hover .toc-section-header {
            background: var(--bg-primary);
            transform: translateX(4px);
            color: var(--primary-blue);
        }

        .toc-number {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 24px;
            height: 24px;
            background: var(--primary-blue);
            color: white;
            border-radius: 50%;
            font-size: 0.75rem;
            font-weight: 600;
            margin-right: 0.75rem;
            flex-shrink: 0;
        }

        .toc-section.active .toc-number {
            background: rgba(255, 255, 255, 0.2);
        }

        .toc-title {
            flex: 1;
            font-weight: 500;
        }

        .toc-toggle {
            font-size: 0.8rem;
            transition: transform 0.2s ease;
            margin-left: 0.5rem;
            color: var(--text-muted);
        }

        .toc-section.active .toc-toggle {
            color: rgba(255, 255, 255, 0.8);
        }

        .toc-subsections {
            list-style: none;
            padding: 0;
            margin: 0;
            max-height: 0;
            overflow: hidden;
            transition: max-height 0.3s ease, padding 0.3s ease;
            background: rgba(0, 0, 0, 0.05);
            border-radius: 0 0 8px 8px;
        }

        .toc-section.expanded .toc-subsections {
            max-height: 500px;
            padding: 0.5rem 0;
        }

        .toc-section.active .toc-subsections {
            background: rgba(255, 255, 255, 0.1);
        }

        .toc-subitem {
            margin: 0;
            transition: all 0.2s ease;
        }

        .toc-subitem a {
            display: block;
            padding: 0.4rem 1rem 0.4rem 2.5rem;
            color: var(--text-muted);
            text-decoration: none;
            font-size: 0.875rem;
            transition: all 0.2s ease;
            border-radius: 4px;
            margin: 0 0.5rem;
        }

        .toc-subitem.level-3 a {
            padding-left: 3rem;
            font-size: 0.825rem;
        }

        .toc-subitem.level-4 a {
            padding-left: 3.5rem;
            font-size: 0.8rem;
        }

        .toc-section.active .toc-subitem a {
            color: rgba(255, 255, 255, 0.8);
        }

        .toc-subitem:hover a,
        .toc-subitem.active a {
            background: var(--bg-primary);
            color: var(--primary-blue);
            transform: translateX(4px);
        }

        .toc-section.active .toc-subitem:hover a,
        .toc-section.active .toc-subitem.active a {
            background: rgba(255, 255, 255, 0.2);
            color: white;
        }

        .content-area {
            flex: 1;
            background: var(--bg-primary);
            overflow-y: auto;
            overflow-x: hidden;
            position: relative;
            transition: background-color 0.3s ease;
            margin-left: 280px;
            height: calc(100vh - 100px);
        }

        .content-wrapper {
            max-width: 800px;
            margin: 0 auto;
            padding: 2rem;
        }

        .section-content {
            display: none;
            animation: fadeIn 0.4s ease-out;
        }

        .section-content.active {
            display: block;
        }

        .pagination {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 3rem;
            padding-top: 2rem;
            border-top: 1px solid var(--border-color);
        }

        .pagination-btn {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            background: var(--primary-blue);
            color: white;
            border: none;
            padding: 0.75rem 1.5rem;
            border-radius: 8px;
            text-decoration: none;
            transition: all 0.2s ease;
            cursor: pointer;
            font-size: 0.9rem;
        }

        .pagination-btn:hover {
            background: var(--secondary-blue);
            transform: translateY(-1px);
            box-shadow: 0 4px 8px rgba(37, 99, 235, 0.3);
        }

        .pagination-btn:disabled {
            background: var(--gray-300);
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
        }

        .pagination-btn:disabled:hover {
            background: var(--gray-300);
        }

        .pagination-info {
            color: var(--text-muted);
            font-size: 0.875rem;
        }

        h1, h2, h3, h4, h5, h6 {
            color: var(--text-primary);
            margin-top: 2rem;
            margin-bottom: 1rem;
            font-weight: 700;
            line-height: 1.3;
        }

        h1 {
            font-size: 2.5rem;
            border-bottom: 3px solid var(--primary-blue);
            padding-bottom: 1rem;
            position: relative;
            margin-top: 0;
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
            .sidebar {
                position: static;
                width: 100%;
                height: auto;
                max-height: 200px;
                padding: 1rem;
                border-right: none;
                border-bottom: 1px solid var(--sidebar-border);
                top: auto;
                left: auto;
            }
            
            .main-container {
                flex-direction: column;
                height: auto;
                overflow: visible;
            }
            
            .content-area {
                margin-left: 0;
                height: auto;
                overflow-y: visible;
            }
            
            .content-wrapper {
                padding: 1rem;
            }
            
            .header-content {
                flex-direction: column;
                gap: 1rem;
            }
            
            .logo-section {
                justify-content: center;
            }
            
            .theme-toggle {
                width: fit-content;
            }

            h1 {
                font-size: 2rem;
            }
            
            pre {
                padding: 1rem;
                font-size: 0.8rem;
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
            --sidebar-bg: #0f172a;
            --sidebar-border: #334155;
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
            --sidebar-bg: #f8fafc;
            --sidebar-border: #e5e7eb;
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
            <div class="logo-section">
                <img src="img/bluePrint_logo/BluePrint_logo_128px.png" alt="BluePrint Logo" class="logo">
                <h1>BluePrint Documentation</h1>
            </div>
            <button class="theme-toggle" onclick="toggleTheme()">🌓 Toggle Theme</button>
        </div>
    </header>
    
    <div class="main-container">
        <aside class="sidebar">
            <h2>Table of Contents</h2>
            <ul class="toc-list">
                ${tableOfContentsHtml}
            </ul>
        </aside>
        
        <main class="content-area">
            <div class="content-wrapper">
                <div id="content-sections">
                    <!-- Sections will be populated by JavaScript -->
                </div>
                
                <div class="pagination">
                    <button class="pagination-btn" id="prev-btn" onclick="previousSection()" disabled>
                        ← Previous
                    </button>
                    <div class="pagination-info">
                        <span id="current-section">1</span> of <span id="total-sections">${sections.length}</span>
                    </div>
                    <button class="pagination-btn" id="next-btn" onclick="nextSection()">
                        Next →
                    </button>
                </div>
            </div>
            
            <footer class="footer">
                <p>BluePrint Programming Language Documentation</p>
                <p>Built with ❤️ using the BluePrint documentation build system</p>
            </footer>
        </main>
    </div>
    
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.8.0/highlight.min.js"></script>
    <script>
        // Documentation sections data
        const sections = ${sectionsData};
        let currentSectionIndex = 0;
        let currentHeadingId = null;
        
        // Initialize the documentation
        function initDocumentation() {
            renderSections();
            showSection(0);
            updatePagination();
            addHeadingIds();
            hljs.highlightAll();
        }
        
        // Add IDs to all headings in the content for navigation
        function addHeadingIds() {
            sections.forEach((section, sectionIndex) => {
                const sectionElement = document.getElementById(\`section-\${sectionIndex}\`);
                if (sectionElement) {
                    const headings = sectionElement.querySelectorAll('h2, h3, h4');
                    
                    section.headings.forEach(headingData => {
                        const heading = Array.from(headings).find(h => 
                            h.textContent.trim() === headingData.text
                        );
                        if (heading) {
                            heading.id = \`section-\${sectionIndex}-\${headingData.id}\`;
                        }
                    });
                }
            });
        }
        
        // Render all sections into the content area
        function renderSections() {
            const contentContainer = document.getElementById('content-sections');
            contentContainer.innerHTML = sections.map((section, index) => \`
                <div class="section-content" id="section-\${index}" data-section="\${index}">
                    \${section.content}
                </div>
            \`).join('');
        }
        
        // Show a specific section
        function showSection(index) {
            // Hide all sections
            document.querySelectorAll('.section-content').forEach(section => {
                section.classList.remove('active');
            });
            
            // Update TOC active state
            document.querySelectorAll('.toc-section').forEach(item => {
                item.classList.remove('active');
            });
            
            // Clear subitem active states
            document.querySelectorAll('.toc-subitem').forEach(item => {
                item.classList.remove('active');
            });
            
            // Show the selected section
            const targetSection = document.getElementById(\`section-\${index}\`);
            const targetTocSection = document.querySelector(\`.toc-section[data-section="\${index}"]\`);
            
            if (targetSection && targetTocSection) {
                targetSection.classList.add('active');
                targetTocSection.classList.add('active');
                currentSectionIndex = index;
                currentHeadingId = null;
                updatePagination();
                
                // Auto-expand the current section in TOC
                targetTocSection.classList.add('expanded');
                
                // Scroll to top of content area
                document.querySelector('.content-area').scrollTop = 0;
                
                // Re-highlight code blocks in the new section
                targetSection.querySelectorAll('pre code').forEach(block => {
                    hljs.highlightElement(block);
                });
            }
        }
        
        // Show a specific section and scroll to a heading
        function showSectionAndHeading(sectionIndex, headingId) {
            // Hide all sections
            document.querySelectorAll('.section-content').forEach(section => {
                section.classList.remove('active');
            });
            
            // Update TOC active state
            document.querySelectorAll('.toc-section').forEach(item => {
                item.classList.remove('active');
            });
            
            // Clear subitem active states
            document.querySelectorAll('.toc-subitem').forEach(item => {
                item.classList.remove('active');
            });
            
            // Show the selected section
            const targetSection = document.getElementById(\`section-\${sectionIndex}\`);
            const targetTocSection = document.querySelector(\`.toc-section[data-section="\${sectionIndex}"]\`);
            
            if (targetSection && targetTocSection) {
                targetSection.classList.add('active');
                targetTocSection.classList.add('active');
                currentSectionIndex = sectionIndex;
                currentHeadingId = headingId;
                updatePagination();
                
                // Auto-expand the current section in TOC
                targetTocSection.classList.add('expanded');
                
                // Re-highlight code blocks in the new section
                targetSection.querySelectorAll('pre code').forEach(block => {
                    hljs.highlightElement(block);
                });
                
                // Wait for the section to be rendered, then scroll to heading
                setTimeout(() => {
                    const headingElement = document.getElementById(\`section-\${sectionIndex}-\${headingId}\`);
                    if (headingElement) {
                        const contentArea = document.querySelector('.content-area');
                        const elementTop = headingElement.offsetTop - 20; // 20px offset for better visibility
                        contentArea.scrollTo({
                            top: elementTop,
                            behavior: 'smooth'
                        });
                        
                        // Highlight the current subitem
                        const activeSubitem = document.querySelector(\`.toc-subitem[data-section="\${sectionIndex}"][data-heading="\${headingId}"]\`);
                        if (activeSubitem) {
                            activeSubitem.classList.add('active');
                        }
                    }
                }, 50); // Reduced timeout for faster response
            }
        }
        
        // Toggle section expansion in TOC
        function toggleSection(sectionIndex) {
            const tocSection = document.querySelector(\`.toc-section[data-section="\${sectionIndex}"]\`);
            if (tocSection) {
                if (tocSection.classList.contains('expanded')) {
                    tocSection.classList.remove('expanded');
                } else {
                    tocSection.classList.add('expanded');
                }
            }
            
            // If this section is not active, show it
            if (currentSectionIndex !== sectionIndex) {
                showSection(sectionIndex);
            }
        }
        
        // Navigate to previous section
        function previousSection() {
            if (currentSectionIndex > 0) {
                showSection(currentSectionIndex - 1);
            }
        }
        
        // Navigate to next section
        function nextSection() {
            if (currentSectionIndex < sections.length - 1) {
                showSection(currentSectionIndex + 1);
            }
        }
        
        // Update pagination buttons and info
        function updatePagination() {
            const prevBtn = document.getElementById('prev-btn');
            const nextBtn = document.getElementById('next-btn');
            const currentSpan = document.getElementById('current-section');
            const totalSpan = document.getElementById('total-sections');
            
            prevBtn.disabled = currentSectionIndex === 0;
            nextBtn.disabled = currentSectionIndex === sections.length - 1;
            
            currentSpan.textContent = currentSectionIndex + 1;
            totalSpan.textContent = sections.length;
        }
        
        // Keyboard navigation
        document.addEventListener('keydown', function(e) {
            if (e.key === 'ArrowLeft' && currentSectionIndex > 0) {
                previousSection();
            } else if (e.key === 'ArrowRight' && currentSectionIndex < sections.length - 1) {
                nextSection();
            }
        });
        
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
        
        // Initialize everything when DOM is loaded
        document.addEventListener('DOMContentLoaded', function() {
            initTheme();
            initDocumentation();
        });
    </script>
</body>
</html>`;

// Write the HTML file
fs.writeFileSync(path.join(buildDir, 'index.html'), htmlPage);

console.log('Documentation built successfully!');
console.log('Open build/index.html in your browser to view the documentation.');
console.log('Or run "npm run serve" to start a local server at http://localhost:3000');
