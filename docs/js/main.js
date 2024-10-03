document.addEventListener('DOMContentLoaded', function() {
	const links = document.querySelectorAll('.sidebar ul li a');
	links.forEach(link => {
		link.addEventListener('click', function(e) {
			e.preventDefault();
			const targetId = this.getAttribute('href').substring(1);
			const targetSection = document.getElementById(targetId);
			targetSection.scrollIntoView({ behavior: 'smooth' });
		});
	});

	const collapsibleLinks = document.querySelectorAll('.collapsible');
	collapsibleLinks.forEach(link => {
		link.addEventListener('click', function(e) {
			e.preventDefault();
			const nestedList = this.nextElementSibling;

			if (nestedList.style.display === 'block') {
				nestedList.style.display = 'none';
				this.classList.remove('active');
			} else {
				nestedList.style.display = 'block';
				this.classList.add('active');
			}
		});
	});
});
