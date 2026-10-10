
// ================= BACKEND CONNECTION =================

async function connectToCBackend() {
    try {
        const response = await fetch("/api/stats");

        if (!response.ok) {
            throw new Error("Backend not responding");
        }

        const data = await response.json();

        console.log("C Backend Connected Successfully");
        console.log(data);

        // Mission Dashboard
        document.getElementById("totalPatients").textContent =
            data.total_patients;

        document.getElementById("emergencyCases").textContent =
            data.emergency_cases;

        document.getElementById("icuCases").textContent =
            data.icu_cases;

        document.getElementById("normalQueue").textContent =
            data.normal_queue;

        document.getElementById("averageAge").textContent =
            data.average_age;

        document.getElementById("averageSatisfaction").textContent =
            data.average_satisfaction + "%";

        document.getElementById("averageStay").textContent =
            data.average_stay + " Days";

        // Other modules
        document.getElementById("patientCountDisplay").textContent =
            data.total_patients;

        document.getElementById("emergencyQueueCount").textContent =
            data.emergency_cases;

        document.getElementById("analyticsPatients").textContent =
            data.total_patients;

        document.getElementById("analyticsEmergency").textContent =
            data.emergency_cases;

        document.getElementById("analyticsStay").textContent =
            data.average_stay + " Days";

    } catch (error) {
        console.log("C Backend Connection Failed");
        console.log(error);
    }
}


// ================= LAUNCH PAGE =================

function showDashboard() {

    document.getElementById("launchPage").style.display = "none";
    document.getElementById("dashboardPage").style.display = "block";

    connectToCBackend();
}


function showLaunch() {

    document.getElementById("dashboardPage").style.display = "none";
    document.getElementById("launchPage").style.display = "flex";
}


// ================= MODULE NAVIGATION =================

function showSection(sectionId) {

    const sections = document.querySelectorAll(".content-section");

    sections.forEach(function(section) {
        section.classList.remove("active");
    });

    const selectedSection = document.getElementById(sectionId);

    if (selectedSection) {
        selectedSection.classList.add("active");
    }
}


// ================= SEARCH PATIENT =================

async function searchPatient() {

    const input = document.getElementById("patientSearchId");
    const result = document.getElementById("searchResult");

    const patientId = input.value.trim();

    if (patientId === "") {

        result.innerHTML = `
            <div class="info-card">
                <h2>Patient Search</h2>
                <p>Please enter a Patient ID.</p>
            </div>
        `;

        return;
    }

    result.innerHTML = `
        <div class="info-card">
            <h2>Searching...</h2>
            <p>Checking patient records from C backend.</p>
        </div>
    `;

    try {

        const response = await fetch(
            "/api/search?id=" + encodeURIComponent(patientId)
        );

        if (!response.ok) {
            throw new Error("Search request failed");
        }

        const data = await response.json();

        console.log("Search Result:", data);

        if (data.found === true) {

            result.innerHTML = `
                <div class="info-card">

                    <h2>Patient Found</h2>

                    <p>
                        <strong>Patient ID:</strong>
                        ${data.patient_id}
                    </p>

                    <p>
                        <strong>Name:</strong>
                        ${data.name}
                    </p>

                    <p>
                        <strong>Age:</strong>
                        ${data.age}
                    </p>

                    <p>
                        <strong>Arrival Date:</strong>
                        ${data.arrival_date}
                    </p>

                    <p>
                        <strong>Departure Date:</strong>
                        ${data.departure_date}
                    </p>

                    <p>
                        <strong>Service:</strong>
                        ${data.service}
                    </p>

                    <p>
                        <strong>Satisfaction:</strong>
                        ${data.satisfaction}
                    </p>

                    <p>
                        <strong>Stay:</strong>
                        ${data.stay_days} days
                    </p>

                </div>
            `;

        } else {

            result.innerHTML = `
                <div class="info-card">

                    <h2>Patient Not Found</h2>

                    <p>
                        No patient found with ID:
                        <strong>${patientId}</strong>
                    </p>

                    <p>
                        Please check the Patient ID and try again.
                    </p>

                </div>
            `;
        }

    } catch (error) {

        console.error("Patient Search Error:", error);

        result.innerHTML = `
            <div class="info-card">

                <h2>Backend Connection Error</h2>

                <p>
                    Unable to connect to the C backend.
                </p>

                <p>
                    Please make sure <strong>server.exe</strong>
                    is running.
                </p>

            </div>
        `;
    }
}