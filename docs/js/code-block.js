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

class InlineCode extends HTMLElement { }

customElements.define('code-block', CodeBlock);
customElements.define('pb-keyword', Keyword);
customElements.define('pb-string', StringElement);
customElements.define('pb-number', NumberElement);
customElements.define('pb-boolean', BooleanElement);
customElements.define('pb-class', Class);
customElements.define('pb-attribute', Attribute);
customElements.define('pb-static', Static);
customElements.define('pb-comment', Comment);
customElements.define('pb-error', Error);
customElements.define('pb-character', Character);

customElements.define('inline-code', InlineCode);
