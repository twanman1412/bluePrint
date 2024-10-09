class CodeBlock extends HTMLElement {
	connectedCallback() {
		this.style.display = 'block';
	}
}

class Keyword extends HTMLElement { }
class StringElement extends HTMLElement { }
class NumberElement extends HTMLElement { }
class BooleanElement extends HTMLElement { }
class Class extends HTMLElement { }
class Attribute extends HTMLElement { }
class Static extends HTMLElement { }
class Comment extends HTMLElement { }
class Error extends HTMLElement { }
class Character extends HTMLElement { }
class Function extends HTMLElement { }

class InlineCode extends HTMLElement { }

class Indent extends HTMLElement { }


customElements.define('code-block', CodeBlock);
customElements.define('bp-keyword', Keyword);
customElements.define('bp-string', StringElement);
customElements.define('bp-number', NumberElement);
customElements.define('bp-boolean', BooleanElement);
customElements.define('bp-class', Class);
customElements.define('bp-attribute', Attribute);
customElements.define('bp-static', Static);
customElements.define('bp-comment', Comment);
customElements.define('bp-error', Error);
customElements.define('bp-character', Character);
customElements.define('bp-function', Function);

customElements.define('inline-code', InlineCode);

customElements.define('bp-indent', Indent);
