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

customElements.define('code-block', CodeBlock);
customElements.define('keyword', Keyword);
customElements.define('string', StringElement);
customElements.define('number', NumberElement);
customElements.define('boolean', BooleanElement);
customElements.define('class', Class);
customElements.define('attribute', Attribute);
customElements.define('static', Static);
